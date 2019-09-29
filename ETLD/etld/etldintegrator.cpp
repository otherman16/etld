#include "etldintegrator.h"

#include "etld/etldclassifier.h"
#include "etld/etldmodel.h"
#include <cstring>
#include <math.h>

EtldIntegrator::EtldIntegrator()
{
    c_thrld = 0.7f;
    overlap_thrld = 0.1f;
    frames_valid = 40;
    a_xy = 0.8f;
    a_wh = 0.5f;
}
EtldIntegrator::~EtldIntegrator()
{
}
void EtldIntegrator::init(const cv::Mat_<uint8_t> & , const etld_object & , const EtldClassifier & , const EtldModel & , const etld_settings & settings)
{
    c_thrld = settings.integrator_settings.c_thrld;
    overlap_thrld = settings.integrator_settings.overlap_thrld;
    frames_valid = settings.integrator_settings.frames_valid;
    a_xy = settings.integrator_settings.a_xy;
    a_wh = settings.integrator_settings.a_wh;
}
void EtldIntegrator::integrate(etld_object & object, const etld_tracker_candidate & tracker_candidate, const etld_detector_candidate * detector_candidates, const int & detector_candidates_num)
{
    clusters = new etld_cluster[detector_candidates_num];
    for(int i = 0; i < detector_candidates_num; ++i)
    {
        clusters[i].idx = new int[detector_candidates_num];
    }
    indexes = new int[detector_candidates_num];
    weights = new float[detector_candidates_num];

    clusterise(detector_candidates, detector_candidates_num);

    bool reinit_filter = !object.valid;
    int prev_x0 = object.window.x;
    int prev_y0 = object.window.y;
    int prev_w  = object.window.width;
    int prev_h  = object.window.height;

    if(tracker_candidate.success) // Если трекер дал валидный результат
    {
        if(clusters_n > 0) // Если есть хотя бы 1 кластер
        {
            // Находятся индексы кластеров, которые далеки от результата трекера и
            // имеют бОльшую похожесть на модель объекта, чем результат трекера
            int cnt = 0;
            for(int i = 0; i < clusters_n; i++)
            {
                float over = overlap(tracker_candidate.window, clusters[i].window);
                if( (over < overlap_thrld) && (clusters[i].C > tracker_candidate.C + 0.1f) )
                {
                    indexes[cnt] = i;
                    ++cnt;
                }
            }
            if(cnt == 1) // Если кластер, далекий от кандидата трекера и имеющий больший коэффициент схожести с моделью, один
            {
                object.valid = true;
                object.tracker_reinit = true;
                object.update = false;
                object.C = clusters[indexes[0]].C;
                object.R = clusters[indexes[0]].R;
                object.window.x = clusters[indexes[0]].window.x;
                object.window.y = clusters[indexes[0]].window.y;
                object.window.width = clusters[indexes[0]].window.width;
                object.window.height = clusters[indexes[0]].window.height;
                reinit_filter = true;
            }
            else // Находятся кластеры, близкие к кандидату трекера
            {
                cnt = 0;
                for(int i = 0; i < clusters_n; i++)
                {
                    float over = overlap(tracker_candidate.window, clusters[i].window);
                    if(over > overlap_thrld)
                    {
                        indexes[cnt] = i;
                        ++cnt;
                    }
                }
                if(cnt > 0) // Если нашлись кластеры, близкие к кандидату трекера
                {
                    float sum_weights_C = 0;
                    float sum_weights_R = 0;
                    for(int i = 0; i < cnt; i++)
                    {
                        weights[i] = clusters[indexes[i]].C;
                        sum_weights_C += weights[i];
                        sum_weights_R += clusters[indexes[i]].R;
                    }
                    if(tracker_candidate.C > c_thrld)
                    {
                        sum_weights_C += tracker_candidate.C;
                    }

                    float x0_C = 0;
                    float y0_C = 0;
                    float w_C = 0;
                    float h_C = 0;
                    for(int i = 0; i < cnt; i++)
                    {
                        x0_C += ( clusters[indexes[i]].window.x * weights[i] );
                        y0_C += ( clusters[indexes[i]].window.y * weights[i] );
                        w_C  += ( clusters[indexes[i]].window.width * weights[i] );
                        h_C  += ( clusters[indexes[i]].window.height * weights[i] );
                    }
                    if(tracker_candidate.C > c_thrld)
                    {
                        x0_C += ( tracker_candidate.window.x * tracker_candidate.C );
                        y0_C += ( tracker_candidate.window.y * tracker_candidate.C );
                        w_C  += ( tracker_candidate.window.width * tracker_candidate.C );
                        h_C  += ( tracker_candidate.window.height * tracker_candidate.C );
                    }

                    x0_C /= sum_weights_C;
                    y0_C /= sum_weights_C;
                    w_C /= sum_weights_C;
                    h_C /= sum_weights_C;

                    object.valid = true;
                    object.tracker_reinit = (tracker_candidate.C > c_thrld) ? false : true;
                    object.update = true;
                    object.C = (tracker_candidate.C > c_thrld) ? (sum_weights_C / float(cnt + 1)) : (sum_weights_C / float(cnt));
                    object.R = sum_weights_R / (cnt);
                    object.window.x = int(roundf(x0_C));
                    object.window.y = int(roundf(y0_C));
                    object.window.width  = int(roundf(w_C));
                    object.window.height  = int(roundf(h_C));
                }
                else
                {
                    object.valid = (tracker_candidate.C > c_thrld) ? true : false;
                    object.tracker_reinit = false;
                    object.update = (tracker_candidate.C > c_thrld) ? true : false;
                    object.C = tracker_candidate.C;
                    object.R = tracker_candidate.R;
                    object.window.x = tracker_candidate.window.x;
                    object.window.y = tracker_candidate.window.y;
                    object.window.width = tracker_candidate.window.width;
                    object.window.height = tracker_candidate.window.height;
                }
            }
        }
        else
        {
            object.valid = (tracker_candidate.C > c_thrld) ? true : false;
            object.tracker_reinit = false;
            object.update = (tracker_candidate.C > c_thrld) ? true : false;
            object.C = tracker_candidate.C;
            object.R = tracker_candidate.R;
            object.window.x = tracker_candidate.window.x;
            object.window.y = tracker_candidate.window.y;
            object.window.width = tracker_candidate.window.width;
            object.window.height = tracker_candidate.window.height;
        }
    }
    else // Если трекер не дал валидный результат
    {
        if(clusters_n == 1) // Если кластер один, то это новая цель, но трекер надо сбросить
        {
            object.valid = true;
            object.tracker_reinit = true;
            object.update = false;
            object.C = clusters[0].C;
            object.R = clusters[0].R;
            object.window.x = clusters[0].window.x;
            object.window.y = clusters[0].window.y;
            object.window.width = clusters[0].window.width;
            object.window.height = clusters[0].window.height;
            reinit_filter = true;
        }
        else // Если кластеров несколько, то цель потеряна
        {
            object.valid = false;
            object.tracker_reinit = false;
            object.update = false;
        }
    }

    // Exponential filter
    if(object.valid)
    {
        if(!reinit_filter)
        {
            object.window.x = int(roundf(prev_x0 * (1 - a_xy) + object.window.x * a_xy));
            object.window.y = int(roundf(prev_y0 * (1 - a_xy) + object.window.y * a_xy));
            object.window.width = int(roundf(prev_w * (1 - a_wh) + object.window.width * a_wh));
            object.window.height = int(roundf(prev_h * (1 - a_wh) + object.window.height * a_wh));
        }
    }

    if(object.window.width < 5) object.window.width = 5;
    else if(object.window.width > ETLD_W - 5) object.window.width = ETLD_W - 5;
    if(object.window.height < 5) object.window.height = 5;
    else if(object.window.height > ETLD_H - 5) object.window.height = ETLD_H - 5;
    if((object.window.x < 5) || (object.window.y < 5) || (object.window.x + object.window.width > ETLD_W - 5) || (object.window.y + object.window.height > ETLD_H - 5))
    {
        object.valid = false;
        object.update = false;
        object.tracker_reinit = false;
    }

    for(int i = 0; i < detector_candidates_num; ++i)
    {
        delete [] clusters[i].idx;
    }
    delete [] clusters;
    delete [] indexes;
    delete [] weights;
}
void EtldIntegrator::clusterise(const etld_detector_candidate * detector_candidates, const int & detector_candidates_num)
{
    // Сброс кластеров
    for(int i = 0; i < detector_candidates_num; i++)
    {
        clusters[i].C = 0;
        clusters[i].R = 0;
        clusters[i].n = 0;
        clusters[i].window.x = 0;
        clusters[i].window.y = 0;
        clusters[i].window.width = 0;
        clusters[i].window.height = 0;
        for(int j = 0; j < detector_candidates_num; j++)
        {
            clusters[i].idx[j] = 0;
        }
    }
    clusters_n = 0;

    // Кластеризация
    if(detector_candidates_num > 1)
    {
        for(int i = 0; i < detector_candidates_num; i++)
        {
            if(detector_candidates[i].C < c_thrld) continue;
            // Для каждой потенциальной цели определяем принадлежит ли она хоть одному существующему кластеру
            bool cluster_found = false;
            for(int j = 0; j < clusters_n; j++)
            {
                bool match = true;
                for(int k = 0; k < clusters[j].n; k++)
                {
                    float over = overlap(detector_candidates[ clusters[j].idx[k] ].window, detector_candidates[i].window);
                    if(over < overlap_thrld)
                    {
                        match = false;
                        break;
                    }
                }
                if(match) // Цель перекрывается со всеми стробами в кластере - добавляем в существующий
                {
                    cluster_found = true;
                    clusters[j].idx[ clusters[j].n ] = i; // Добавляем индекс кандидата в кластер
                    clusters[j].n++; // Увеличиваем количество целей в кластере
                    break; // Выходим из цикла по кластерам, переходим к след кандидату
                }
            }
            if(!cluster_found) // Создаем новый кластер и добавляем в него цель
            {
                clusters[clusters_n].idx[0] = i; // Добавляем индекс кандидата в кластер
                clusters[clusters_n].n = 1; // Увеличиваем количество целей в кластере
                ++clusters_n; // Увеличиваем количество кластеров
            }
        }
    }
    else if(detector_candidates_num == 1) // Кандидат один
    {
        if(detector_candidates[0].C > c_thrld)
        {
            clusters_n = 1;
            clusters[0].C = detector_candidates[0].C;
            clusters[0].R = detector_candidates[0].R;
            clusters[0].n = 1;
            clusters[0].idx[0] = 0;
            clusters[0].window.x = detector_candidates[0].window.x;
            clusters[0].window.y = detector_candidates[0].window.y;
            clusters[0].window.width = detector_candidates[0].window.width;
            clusters[0].window.height = detector_candidates[0].window.height;
        }
    }

    // Необходимо каждому из кластеров поставить в соответствие общий строб
    for(int i = 0; i < clusters_n; i++)
    {
        float sum_weights_R = 0;
        float sum_weights_C = 0;
        for(int k = 0; k < clusters[i].n; k++)
        {
            weights[k] = detector_candidates[ clusters[i].idx[k] ].R;
            sum_weights_R += weights[k];
            sum_weights_C += detector_candidates[ clusters[i].idx[k] ].C;
        }

        float x0_R = 0;
        float y0_R = 0;
        float w_R = 0;
        float h_R = 0;
        for(int k = 0; k < clusters[i].n; k++)
        {
            x0_R += ( detector_candidates[ clusters[i].idx[k] ].window.x * weights[k] );
            y0_R += ( detector_candidates[ clusters[i].idx[k] ].window.y * weights[k] );
            w_R  += ( detector_candidates[ clusters[i].idx[k] ].window.width * weights[k] );
            h_R  += ( detector_candidates[ clusters[i].idx[k] ].window.height * weights[k] );
        }
        x0_R /= sum_weights_R;
        y0_R /= sum_weights_R;
        w_R /= sum_weights_R;
        h_R /= sum_weights_R;
        clusters[i].C = sum_weights_C / float(clusters[i].n);
        clusters[i].C = int32_t(clusters[i].C * 100.0f) / 100.0f;
        clusters[i].R = sum_weights_R / float(clusters[i].n);
        clusters[i].R = int32_t(clusters[i].R * 100.0f) / 100.0f;
        clusters[i].window.x = int(roundf(x0_R));
        clusters[i].window.y = int(roundf(y0_R));
        clusters[i].window.width  = int(roundf(w_R));
        clusters[i].window.height  = int(roundf(h_R));
    }
}
float EtldIntegrator::overlap(const cv::Rect_<int> & p0, const cv::Rect_<int> & p1)
{
    cv::Rect_<int> overlap_roi = p0 & p1;
    float s1 = float(p0.area());
    float s2 = float(p1.area());
    float s = float(overlap_roi.area());
    if(s == 0.0f) return 0.0f;
    float o = (s1 < s2) ? (s1 / s) : (s2 / s);
    return o;
}
