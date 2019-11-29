#ifndef ETLDPARAMS_H
#define ETLDPARAMS_H

#include "etld_global.h"

namespace cv
{
namespace etld
{
typedef struct {
    int ex_w;
    int ex_h;
    int ex_n;
    int ex_const_n;
} t_model_settings;
typedef struct {
    int fern_n;
    int fern_k;
} t_classifier_settings;
typedef struct {
    int grid_step;
    float r_thrld;
    int max_candidates_num;
    float detect_scales[DETECT_SCALES];
    float min_scale;
    float max_scale;
} t_detector_settings;
typedef struct {
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
} t_tracker_settings;
typedef struct {
    float overlap_thrld;
    float c_thrld;
    int frames_valid;
    float a_xy;
    float a_wh;
} t_integrator_settings;
typedef struct {
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
} t_learning_settings;

class ETLDParams
{
public:
    ETLDParams();

    void read(const FileNode & fn);
    void write(FileStorage & fs) const;

private:
    void read_model_settings(const FileNode & fn);
    void read_classifier_settings(const FileNode & fn);
    void read_detector_settings(const FileNode & fn);
    void read_tracker_settings(const FileNode & fn);
    void read_integrator_settings(const FileNode & fn);
    void read_learning_settings(const FileNode & fn);

    void write_model_settings(FileStorage & fs) const;
    void write_classifier_settings(FileStorage & fs) const;
    void write_detector_settings(FileStorage & fs) const;
    void write_tracker_settings(FileStorage & fs) const;
    void write_integrator_settings(FileStorage & fs) const;
    void write_learning_settings(FileStorage & fs) const;

public:
    t_model_settings model_settings;
    t_classifier_settings classifier_settings;
    t_detector_settings detector_settings;
    t_tracker_settings tracker_settings;
    t_integrator_settings integrator_settings;
    t_learning_settings learning_settings;
};

}
}

#endif // ETLDPARAMS_H
