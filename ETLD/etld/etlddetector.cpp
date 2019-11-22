#include "etlddetector.h"

#include "etld/etldclassifier.h"
#include "etld/etldmodel.h"
#include "etld/etldimage.h"
#include "etld/etldsimpleimage.h"

#include <math.h>
#include <cstring>

namespace cv
{
EtldDetector::EtldDetector()
{
    grid_step = 2;
    R_thrld = 0.7f;
    D_thrld = 0;
    max_candidates_num = 8;
}
EtldDetector::~EtldDetector()
{
}
void EtldDetector::init(const cv::Mat_<uint8_t> & frame, const etld_object & object, const EtldClassifier & classifier, const EtldModel & , const etld_settings & settings)
{
    const int w = object.window.width;
    const int h = object.window.height;
    EtldImage obj(frame, object.window);
    D_thrld = obj.D() >> 1;
    R_thrld = settings.detector_settings.r_thrld;
    grid_step = int(settings.detector_settings.grid_step);
    max_candidates_num = int(settings.detector_settings.max_candidates_num);
    memcpy(detect_scales, settings.detector_settings.detect_scales, DETECT_SCALES * sizeof(float));
    min_scale = settings.detector_settings.min_scale;
    max_scale = settings.detector_settings.max_scale;
    for(uint32_t scale_idx = 0; scale_idx < DETECT_SCALES; ++scale_idx)
    {
        w_scaled[scale_idx] = int(roundf(float(w * detect_scales[scale_idx])));
        h_scaled[scale_idx] = int(roundf(float(h * detect_scales[scale_idx])));
    }
    classifier.construct_detect_fern(w, h, detect_scales, frame.cols);
}
int EtldDetector::detect(const cv::Mat_<uint8_t> & frame, const etld_object & object, etld_detector_candidate * candidates, const EtldClassifier & classifier, const EtldModel & model)
{
    // Update scan window scales
    if(object.update)
    {
        int x = object.window.x;
        int y = object.window.y;
        int w = object.window.width;
        int h = object.window.height;
        const int w_min = int(roundf(w_scaled[0] * min_scale));
        const int w_max = int(roundf(w_scaled[0] * max_scale));
        const int h_min = int(roundf(h_scaled[0] * min_scale));
        const int h_max = int(roundf(h_scaled[0] * max_scale));
        if( (w >= w_max) || (h >= h_max) || (w <= w_min) || (h <= h_min) )
        {
            w = (w >= w_max) ? w_max : (w <= w_min) ? w_min : w;
            h = (h >= h_max) ? h_max : (h <= h_min) ? h_min : h;
            EtldImage obj(frame, x, y, w, h);
            D_thrld = obj.D() >> 1;
            for(uint32_t scale_idx = 0; scale_idx < DETECT_SCALES; ++scale_idx)
            {
                w_scaled[scale_idx] = int(roundf(float(w * detect_scales[scale_idx])));
                h_scaled[scale_idx] = int(roundf(float(h * detect_scales[scale_idx])));
            }
            classifier.construct_detect_fern(w, h, detect_scales, frame.cols);
        }
    }
    // Generate scan grid
    int grid_n = frame.cols * frame.rows / grid_step / grid_step;
    cv::Point2i * grid = new cv::Point2i[grid_n];
    const int x_fin = frame.cols - w_scaled[0] - 1;
    const int y_fin = frame.rows - h_scaled[0] - 1;
    int grid_i = 0;
    for(int y = 0; y < y_fin; y += grid_step)
    {
        for(int x = 0; x < x_fin; x += grid_step)
        {
            grid[grid_i].x = x;
            grid[grid_i].y = y;
            ++grid_i;
        }
    }
    grid_n = grid_i;
    // Scan frame
    int candidates_num = 0;
    float R_min = R_thrld;
    int idx_R_min = 0;
    EtldSimpleImage obj(frame, 0, 0, w_scaled[0], h_scaled[0]);
    for(uint32_t scale_idx = 0; scale_idx < DETECT_SCALES; ++scale_idx)
    {
        obj.set_size(w_scaled[scale_idx], h_scaled[scale_idx]);
        for(int i = 0; i < grid_n; ++i)
        {
            if(((grid[i].x + w_scaled[scale_idx]) >= frame.cols) || ((grid[i].y + h_scaled[scale_idx]) >= frame.rows)) continue;
            obj.set_offset(grid[i].x, grid[i].y);
            float R = classifier.detect_R(obj, scale_idx);
            if(R > R_min)
            {
//                if(obj.D() > D_thrld)
                {
                    if(candidates_num == max_candidates_num)
                    {
                        candidates[idx_R_min].R = R;
                        candidates[idx_R_min].window.x = grid[i].x;
                        candidates[idx_R_min].window.y = grid[i].y;
                        candidates[idx_R_min].window.width = w_scaled[scale_idx];
                        candidates[idx_R_min].window.height = h_scaled[scale_idx];
                        R_min = candidates[0].R;
                        idx_R_min = 0;
                        for(int k = 1; k < candidates_num; ++k)
                        {
                            if(candidates[k].R < R_min)
                            {
                                R_min = candidates[k].R;
                                idx_R_min = k;
                            }
                        }
                    }
                    else
                    {
                        candidates[candidates_num].R = R;
                        candidates[candidates_num].window.x = grid[i].x;
                        candidates[candidates_num].window.y = grid[i].y;
                        candidates[candidates_num].window.width = w_scaled[scale_idx];
                        candidates[candidates_num].window.height = h_scaled[scale_idx];
                        ++candidates_num;
                        if(candidates_num == max_candidates_num)
                        {
                            R_min = candidates[0].R;
                            idx_R_min = 0;
                            for(int k = 1; k < candidates_num; ++k)
                            {
                                if(candidates[k].R < R_min)
                                {
                                    R_min = candidates[k].R;
                                    idx_R_min = k;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    for(int i = 0; i < candidates_num; ++i)
    {
        EtldImage obj(frame, candidates[i].window);
        candidates[i].C = model.C(obj);
        candidates[i].C = (int32_t(candidates[i].C * 100.0f)) / 100.0f;
        candidates[i].R = (int32_t(candidates[i].R * 100.0f)) / 100.0f;
    }
    delete [] grid;
    return candidates_num;
}
}
