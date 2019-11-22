#ifndef ETLD_GLOBAL_H
#define ETLD_GLOBAL_H

#include <opencv2/opencv.hpp>

#include <stdint.h>

#define DETECT_SCALES       (3)
#define LEARN_SCALES        (3)
#define LEARN_ANGLES        (3)

#define ETLD_W (320)
#define ETLD_H (240)
#define ETLD_N (76800)
namespace cv
{
namespace etld
{

#ifdef WIN32
#pragma pack(push, 1)
typedef struct
#else
typedef struct __attribute__((packed))
#endif
_etld_model_settings {
    int ex_w;
    int ex_h;
    int ex_n;
    int ex_const_n;
} etld_model_settings;
#ifdef WIN32
#pragma pack(pop)
#endif
#ifdef WIN32
#pragma pack(push, 1)
typedef struct
#else
typedef struct __attribute__((packed))
#endif
_etld_classifier_settings {
    int fern_n;
    int fern_k;
} etld_classifier_settings;
#ifdef WIN32
#pragma pack(pop)
#endif
#ifdef WIN32
#pragma pack(push, 1)
typedef struct
#else
typedef struct __attribute__((packed))
#endif
_etld_detector_settings {
    int grid_step;
    float r_thrld;
    int max_candidates_num;
    float detect_scales[DETECT_SCALES];
    float min_scale;
    float max_scale;
} etld_detector_settings;
#ifdef WIN32
#pragma pack(pop)
#endif
#ifdef WIN32
#pragma pack(push, 1)
typedef struct
#else
typedef struct __attribute__((packed))
#endif
_etld_tracker_settings {
    int pyramid_levels;
    int win_sz;
    int npts_width;
    int npts_height;
    int nmax_iter;
    float min_delta;
    int variance_thrld_div;
    int subgrid_sz;
    float max_fb_err_coeff;
    float min_part_for_scale;
    bool use_fast;
} etld_tracker_settings;
#ifdef WIN32
#pragma pack(pop)
#endif
#ifdef WIN32
#pragma pack(push, 1)
typedef struct
#else
typedef struct __attribute__((packed))
#endif
_etld_integrator_settings {
    float overlap_thrld;
    float c_thrld;
    int frames_valid;
    float a_xy;
    float a_wh;
} etld_integrator_settings;
#ifdef WIN32
#pragma pack(pop)
#endif
#ifdef WIN32
#pragma pack(push, 1)
typedef struct
#else
typedef struct __attribute__((packed))
#endif
_etld_learning_settings {
    bool reaim_en;
    int reaim_D_thrld_div;
    bool fast_update;
    float neg_r_min;
    float neg_r_max;
    float pos_r_min;
    float pos_r_max;
    float neg_c_min;
    float neg_c_max;
    float pos_c_min;
    float pos_c_max;
    float overlap_thrld;
    float learn_scales[LEARN_SCALES];
    float learn_angles[LEARN_ANGLES];
} etld_learning_settings;
#ifdef WIN32
#pragma pack(pop)
#endif
#ifdef WIN32
#pragma pack(push, 1)
typedef struct
#else
typedef struct __attribute__((packed))
#endif
_etld_settings {
    etld_model_settings model_settings;
    etld_classifier_settings classifier_settings;
    etld_detector_settings detector_settings;
    etld_tracker_settings tracker_settings;
    etld_integrator_settings integrator_settings;
    etld_learning_settings learning_settings;
} etld_settings;
#ifdef WIN32
#pragma pack(pop)
#endif
typedef struct
_etld_object {
    bool valid;
    bool tracker_reinit;
    bool update;
    float C;
    float R;
    cv::Rect_<int> window;
} etld_object;
typedef struct
_etld_tracker_candidate {
    bool success;
    float C;
    float R;
    cv::Rect_<int> window;
} etld_tracker_candidate;
typedef struct
_etld_detector_candidate {
    float C;
    float R;
    cv::Rect_<int> window;
} etld_detector_candidate;
typedef struct
_etld_cluster {
    float C;
    float R;
    int n;
    cv::Rect_<int> window;
    int * idx;
} etld_cluster;

}
}

#endif // ETLD_GLOBAL_H
