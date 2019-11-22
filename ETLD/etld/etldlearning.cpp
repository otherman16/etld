#include "etldlearning.h"

#include "etld/etldimage.h"
#include "etld/etldclassifier.h"
#include "etld/etldmodel.h"

#include <math.h>
#include <cstring>

namespace cv
{
EtldLearning::EtldLearning()
{
    fast_update = false;
    m_pos_num = 0;
    m_neg_num = 0;
    c_pos_num = 0;
    c_neg_num = 0;
    neg_r_min = 0.5f;
    neg_r_max = 0.8f;
    pos_r_min = 0.2f;
    pos_r_max = 0.7f;
    neg_c_min = 0.6f;
    neg_c_max = 0.9f;
    pos_c_min = 0.2f;
    pos_c_max = 0.7f;
    overlap_thrld = 0.125f;
    detector_grid_step = 2;
}
EtldLearning::~EtldLearning()
{
}
void EtldLearning::init(const cv::Mat_<uint8_t> & frame, const etld_object & object, EtldClassifier & classifier, EtldModel & model, const etld_settings & settings)
{
    const int w  = object.window.width;
    const int h  = object.window.height;
    const int x0 = object.window.x;
    const int y0 = object.window.y;
    const int W  = frame.cols;
    const int H  = frame.rows;
    EtldImage obj(frame, object.window);
    fast_update = settings.learning_settings.fast_update;
    neg_r_min = settings.learning_settings.neg_r_min;
    neg_r_max = settings.learning_settings.neg_r_max;
    pos_r_min = settings.learning_settings.pos_r_min;
    pos_r_max = settings.learning_settings.pos_r_max;
    neg_c_min = settings.learning_settings.neg_c_min;
    neg_c_max = settings.learning_settings.neg_c_max;
    pos_c_min = settings.learning_settings.pos_c_min;
    pos_c_max = settings.learning_settings.pos_c_max;
    overlap_thrld = settings.learning_settings.overlap_thrld;
    detector_grid_step = settings.detector_settings.grid_step;
    memcpy(learn_scales, settings.learning_settings.learn_scales, LEARN_SCALES * sizeof(float));
    memcpy(learn_angles, settings.learning_settings.learn_angles, LEARN_ANGLES * sizeof(float));
    //=======================================================================================
    //====== Инициализация Модели и Классификатора
    //=======================================================================================
    classifier.init(frame, object, settings);
    model.init(frame, object, settings);
    //=======================================================================================
    //====== Рассчитать Fern для нового окна
    //=======================================================================================
    classifier.construct_learn_fern(w, h, learn_scales);
    //=======================================================================================
    //====== Определение пороговых значений дисперсии
    //=======================================================================================
    int D_obj = obj.D();
    int D_thrld = D_obj >> 1;
    //=======================================================================================
    //====== Формирование таблицы сдвигов
    //=======================================================================================
    int shifts_n = 3 * 3;
    const int shifts_x_step = (detector_grid_step > 1) ? (detector_grid_step / 2) : 1;
    const int shifts_y_step = (detector_grid_step > 1) ? (detector_grid_step / 2) : 1;
    cv::Point2i * shifts = new cv::Point2i[shifts_n];
    int shifts_i = 0;
    for(int y = y0 - shifts_y_step; y <= y0 + shifts_y_step; y += shifts_y_step)
    {
        for(int x = x0 - shifts_x_step; x <= x0 + shifts_x_step; x += shifts_x_step)
        {
            shifts[shifts_i].x = x;
            shifts[shifts_i].y = y;
            ++shifts_i;
        }
    }
    shifts_n = shifts_i;
    //=======================================================================================
    //====== Формирование таблицы размеров
    //=======================================================================================
    int * w_scaled = new int[LEARN_SCALES];
    int * h_scaled = new int[LEARN_SCALES];
    for(uint32_t scale_idx = 0; scale_idx < LEARN_SCALES; ++scale_idx)
    {
        w_scaled[scale_idx] = int(roundf(float(w * learn_scales[scale_idx])));
        h_scaled[scale_idx] = int(roundf(float(h * learn_scales[scale_idx])));
    }
    //=======================================================================================
    //====== Добавление положительных примеров
    //=======================================================================================
    m_pos_num = 0;
    c_pos_num = 0;
    for(int i = 0; i < shifts_n; ++i)
    {
        if(((shifts[i].x + w) >= W) || ((shifts[i].y + h) >= H)) continue;
        EtldImage copy_obj(frame, shifts[i].x, shifts[i].y, w, h);
        // Масштабированные примеры
        for(uint32_t j = 0; j < LEARN_SCALES; ++j)
        {
            if(((shifts[i].x + w_scaled[j]) >= W) || ((shifts[i].y + h_scaled[j]) >= H)) continue;
            EtldImage transformed_obj(w_scaled[j], h_scaled[j]);
            copy_obj.bilin_scale(transformed_obj);
            classifier.add_pos_ex(transformed_obj, j);
            model.add_pos_ex(transformed_obj);
            ++c_pos_num;
            ++m_pos_num;
        }
        // Повернутые примеры
        for(uint32_t j = 0; j < LEARN_ANGLES; ++j)
        {
            EtldImage transformed_obj(w, h);
            copy_obj.bilin_rotate(transformed_obj, learn_angles[j]);
            classifier.add_pos_ex(transformed_obj);
            model.add_pos_ex(transformed_obj);
            ++c_pos_num;
            ++m_pos_num;
        }
    }
    //=======================================================================================
    //====== Формирование сетки обхода кадра для добавления отрицательных примеров
    //=======================================================================================
    const int x_start = 8;
    const int y_start = 8;
    const int x_fin = W - w - 8;
    const int y_fin = H - h - 8;
    const int x_step = detector_grid_step * 4;
    const int y_step = detector_grid_step * 4;
    int grid_n = ( (x_fin - x_start) / x_step + 1 ) * ( (y_fin - y_start + 1) / y_step + 1);
    cv::Point2i * grid = new cv::Point2i[grid_n];
    int grid_i = 0;
    for(int y = y_start; y < y_fin; y += y_step)
    {
        for(int x = x_start; x < x_fin; x += x_step)
        {
            grid[grid_i].x = x;
            grid[grid_i].y = y;
            ++grid_i;
        }
    }
    grid_n = grid_i;
    //=======================================================================================
    //====== Добавление отрицательных примеров
    //=======================================================================================
    m_neg_num = 0;
    c_neg_num = 0;
    for(int i = 0; i < grid_n; ++i)
    {
        if(((grid[i].x + w) >= W) || ((grid[i].y + h) >= H)) continue;
        EtldImage copy_obj(frame, grid[i].x, grid[i].y, w, h);
        float O = copy_obj.O(obj);
        if( O < overlap_thrld )
        {
            int D = copy_obj.D();
            if(D > D_thrld)
            {
                classifier.add_neg_ex(copy_obj);
                ++c_neg_num;
            }
            if(m_neg_num < m_pos_num)
            {
                if(D > D_thrld)
                {
                    model.add_neg_ex(copy_obj);
                    ++m_neg_num;
                }
            }
        }
    }
    //=======================================================================================
    //====== Обновление классификатора
    //=======================================================================================
    classifier.update();
    //=======================================================================================
    //====== Освобождение вспомогательных ресурсов
    //=======================================================================================
    delete [] shifts;
    delete [] w_scaled;
    delete [] h_scaled;
    delete [] grid;
}
void EtldLearning::update(const cv::Mat_<uint8_t> & frame, const etld_object & object, EtldClassifier & classifier, EtldModel & model)
{
    const int w  = object.window.width;
    const int h  = object.window.height;
    const int x0 = object.window.x;
    const int y0 = object.window.y;
    const int W  = frame.cols;
    const int H  = frame.rows;
    EtldImage obj(frame, x0, y0, w, h);
    //=======================================================================================
    //====== Рассчитать Fern для нового окна
    //=======================================================================================
    classifier.construct_learn_fern(w, h, learn_scales);
    //=======================================================================================
    //====== Определение пороговых значений дисперсии
    //=======================================================================================
    int D_obj = obj.D();
    int D_thrld = D_obj >> 1;
    //=======================================================================================
    //====== Добавление положительных примеров
    //=======================================================================================
    if(fast_update)
    {
//        float R = classifier.learn_R(obj);
//        if( (pos_r_min < R) && (R < pos_r_max) )
        {
            classifier.add_pos_ex(obj);
            ++c_pos_num;
        }
        float C = model.C(obj);
        if( (pos_c_min < C) && (C < pos_c_max) )
        {
            model.add_pos_ex(obj);
            ++m_pos_num;
        }
    }
    else
    {
        //=======================================================================================
        //====== Формирование таблицы размеров
        //=======================================================================================
        int * w_scaled = new int[LEARN_SCALES];
        int * h_scaled = new int[LEARN_SCALES];
        for(uint32_t scale_idx = 0; scale_idx < LEARN_SCALES; ++scale_idx)
        {
            w_scaled[scale_idx] = int(roundf(float(w * learn_scales[scale_idx])));
            h_scaled[scale_idx] = int(roundf(float(h * learn_scales[scale_idx])));
        }
        //=======================================================================================
        //====== Добавление положительных примеров
        //=======================================================================================
        // Масштабированные примеры
        for(uint32_t j = 0; j < LEARN_SCALES; ++j)
        {
            if(((x0 + w_scaled[j]) >= W) || ((y0 + h_scaled[j]) >= H)) continue;
            EtldImage transformed_obj(w_scaled[j], h_scaled[j]);
            obj.bilin_scale(transformed_obj);
//            float R = classifier.learn_R(transformed_obj, j);
//            if( (pos_r_min < R) && (R < pos_r_max) )
            {
                classifier.add_pos_ex(transformed_obj, j);
                ++c_pos_num;
            }
            float C = model.C(transformed_obj);
            if( (pos_c_min < C) && (C < pos_c_max) )
            {
                model.add_pos_ex(transformed_obj);
                ++m_pos_num;
            }
        }
        // Повернутые примеры
        for(uint32_t j = 0; j < LEARN_ANGLES; ++j)
        {
            EtldImage transformed_obj(w, h);
            obj.bilin_rotate(transformed_obj, learn_angles[j]);
//            float R = classifier.learn_R(transformed_obj);
//            if( (pos_r_min < R) && (R < pos_r_max) )
            {
                classifier.add_pos_ex(transformed_obj);
                ++c_pos_num;
            }
            float C = model.C(transformed_obj);
            if( (pos_c_min < C) && (C < pos_c_max) )
            {
                model.add_pos_ex(transformed_obj);
                ++m_pos_num;
            }
        }
        delete [] w_scaled;
        delete [] h_scaled;
    }
    //=======================================================================================
    //====== Формирование сетки обхода кадра для добавления отрицательных примеров
    //=======================================================================================
    int x_start;
    int y_start;
    int x_fin;
    int y_fin;
    int x_step;
    int y_step;
    if(fast_update)
    {
        x_start = 8;
        y_start = 8;
        x_fin = W - w - 8;
        y_fin = H - h - 8;
        x_step = detector_grid_step * 8;
        y_step = detector_grid_step * 8;
    }
    else
    {
        x_start = 8;
        y_start = 8;
        x_fin = W - w - 8;
        y_fin = H - h - 8;
        x_step = detector_grid_step * 4;
        y_step = detector_grid_step * 4;
    }
    int grid_n = ( (x_fin - x_start) / x_step + 1 ) * ( (y_fin - y_start + 1) / y_step + 1);
    cv::Point2i * grid = new cv::Point2i[grid_n];
    int grid_i = 0;
    for(int y = y_start; y < y_fin; y += y_step)
    {
        for(int x = x_start; x < x_fin; x += x_step)
        {
            grid[grid_i].x = x;
            grid[grid_i].y = y;
            ++grid_i;
        }
    }
    grid_n = grid_i;
    //=======================================================================================
    //====== Добавление отрицательных примеров
    //=======================================================================================
    for(int i = 0; i < grid_n; ++i)
    {
        if(((grid[i].x + w) >= W) || ((grid[i].y + h) >= H)) continue;
        EtldImage copy_obj(frame, grid[i].x, grid[i].y, w, h);
        float O = copy_obj.O(obj);
        if( O < overlap_thrld )
        {
//            int D = copy_obj.D();
//            if(c_neg_num < c_pos_num)
            {
                float R = classifier.learn_R(copy_obj);
                if( (neg_r_min < R) && (R < neg_r_max) )
                {
//                    if(D > D_thrld)
                    {
                        classifier.add_neg_ex(copy_obj);
                        ++c_neg_num;
                    }
                }
            }
            if(m_neg_num < m_pos_num)
            {
                float C = model.C(copy_obj);
                if( (neg_c_min < C) && (C < neg_c_max) )
                {
//                    if(D > D_thrld)
                    {
                        model.add_neg_ex(copy_obj);
                        ++m_neg_num;
                    }
                }
            }
        }
    }
    //=======================================================================================
    //====== Обновление классификатора
    //=======================================================================================
    classifier.update();
    //=======================================================================================
    //====== Освобождение вспомогательных ресурсов
    //=======================================================================================
    delete [] grid;
}
}
