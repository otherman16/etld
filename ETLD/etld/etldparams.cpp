#include "etldparams.h"

namespace cv
{
namespace etld
{

ETLDParams::ETLDParams()
{
    model_settings.ex_w = 15;
    model_settings.ex_h = 15;
    model_settings.ex_n = 100;
    model_settings.ex_const_n = 50;

    classifier_settings.fern_n = 20;
    classifier_settings.fern_k = 10;

    detector_settings.grid_step = 2;
    detector_settings.r_thrld = 0.7f;
    detector_settings.max_candidates_num = 8;
    detector_settings.detect_scales[0] = 1.00f;
    detector_settings.detect_scales[1] = 0.80f;
    detector_settings.detect_scales[2] = 1.20f;
    detector_settings.min_scale        = 0.75f;
    detector_settings.max_scale        = 1.50f;

    tracker_settings.pyramid_levels        = 3;
    tracker_settings.win_sz                = 5;
    tracker_settings.npts_width            = 10;
    tracker_settings.npts_height           = 10;
    tracker_settings.nmax_iter             = 20;
    tracker_settings.min_delta             = 0.03f;
    tracker_settings.variance_thrld_div    = 20;
    tracker_settings.subgrid_sz            = 7;
    tracker_settings.max_fb_err_coeff      = 0.05f;
    tracker_settings.min_part_for_scale    = 0.1f;
    tracker_settings.use_fast              = false;

    integrator_settings.c_thrld = 0.6f;
    integrator_settings.overlap_thrld = 0.5;
    integrator_settings.frames_valid = 40;
    integrator_settings.a_xy = 0.9f;
    integrator_settings.a_wh = 0.7f;

    learning_settings.reaim_en = true;
    learning_settings.reaim_D_thrld_div = 400;
    learning_settings.fast_update       = false;
    learning_settings.neg_r_min         = 0.1f;
    learning_settings.neg_r_max         = detector_settings.r_thrld + 0.1f;//0.6f;
    learning_settings.pos_r_min         = detector_settings.r_thrld - 0.1f;//0.6f;
    learning_settings.pos_r_max         = 0.9f;
    learning_settings.neg_c_min         = integrator_settings.c_thrld;//0.7f;
    learning_settings.neg_c_max         = 0.9f;
    learning_settings.pos_c_min         = 0.1f;
    learning_settings.pos_c_max         = integrator_settings.c_thrld + 0.1f;//0.7f;
    learning_settings.overlap_thrld     = 0.5f;
    learning_settings.learn_scales[0]   = 1.00f;
    learning_settings.learn_scales[1]   = 0.80f;
    learning_settings.learn_scales[2]   = 1.20f;
    learning_settings.learn_angles[0]   = 0.00f;
    learning_settings.learn_angles[1]   = -2.5f;
    learning_settings.learn_angles[2]   = 2.5f;
}

void ETLDParams::read(const FileNode & fn)
{
    read_model_settings(fn["model_settings"]);
    read_classifier_settings(fn["classifier_settings"]);
    read_detector_settings(fn["detector_settings"]);
    read_tracker_settings(fn["tracker_settings"]);
    read_integrator_settings(fn["integrator_settings"]);
    read_learning_settings(fn["learning_settings"]);
}
void ETLDParams::write(FileStorage & fs) const
{
    fs << "etld_settings" << "{";
    write_model_settings(fs);
    write_classifier_settings(fs);
    write_detector_settings(fs);
    write_tracker_settings(fs);
    write_integrator_settings(fs);
    write_learning_settings(fs);
    fs << "}";
}

void ETLDParams::read_model_settings(const FileNode & fn)
{
    model_settings.ex_w = int(fn["ex_w"].real());
    model_settings.ex_h = int(fn["ex_h"].real());
    model_settings.ex_n = int(fn["ex_n"].real());
    model_settings.ex_const_n = int(fn["ex_const_n"].real());
}
void ETLDParams::read_classifier_settings(const FileNode & fn)
{
    classifier_settings.fern_n = int(fn["fern_n"].real());
    classifier_settings.fern_k = int(fn["fern_k"].real());
}
void ETLDParams::read_detector_settings(const FileNode & fn)
{
    detector_settings.grid_step = int(fn["grid_step"].real());
    detector_settings.r_thrld = float(fn["r_thrld"].real());
    detector_settings.max_candidates_num = int(fn["max_candidates_num"].real());
    detector_settings.detect_scales[0] = float(fn["detect_scales_0"].real());
    detector_settings.detect_scales[1] = float(fn["detect_scales_1"].real());
    detector_settings.detect_scales[2] = float(fn["detect_scales_2"].real());
    detector_settings.min_scale = float(fn["min_scale"].real());
    detector_settings.max_scale = float(fn["max_scale"].real());
}
void ETLDParams::read_tracker_settings(const FileNode & fn)
{
    tracker_settings.pyramid_levels = int(fn["pyramid_levels"].real());
    tracker_settings.win_sz = int(fn["win_sz"].real());
    tracker_settings.npts_width = int(fn["npts_width"].real());
    tracker_settings.npts_height = int(fn["npts_height"].real());
    tracker_settings.nmax_iter = int(fn["nmax_iter"].real());
    tracker_settings.min_delta = float(fn["min_delta"].real());
    tracker_settings.variance_thrld_div = int(fn["variance_thrld_div"].real());
    tracker_settings.subgrid_sz = int(fn["subgrid_sz"].real());
    tracker_settings.max_fb_err_coeff = float(fn["max_fb_err_coeff"].real());
    tracker_settings.min_part_for_scale = float(fn["min_part_for_scale"].real());
    tracker_settings.use_fast = bool(fn["use_fast"].real());
}
void ETLDParams::read_integrator_settings(const FileNode & fn)
{
    integrator_settings.c_thrld = float(fn["c_thrld"].real());
    integrator_settings.overlap_thrld = float(fn["overlap_thrld"].real());
    integrator_settings.frames_valid = int(fn["frames_valid"].real());
    integrator_settings.a_xy = float(fn["a_xy"].real());
    integrator_settings.a_wh = float(fn["a_wh"].real());
}
void ETLDParams::read_learning_settings(const FileNode & fn)
{
    learning_settings.reaim_en = bool(fn["reaim_en"].real());
    learning_settings.reaim_D_thrld_div = int(fn["reaim_D_thrld_div"].real());
    learning_settings.fast_update = bool(fn["fast_update"].real());
    learning_settings.neg_r_min = float(fn["neg_r_min"].real());
    learning_settings.neg_r_max = detector_settings.r_thrld + 0.1f;//float(fn["neg_r_max"].real());
    learning_settings.pos_r_min = detector_settings.r_thrld - 0.1f;//float(fn["pos_r_min"].real());
    learning_settings.pos_r_max = float(fn["pos_r_max"].real());
    learning_settings.neg_c_min = integrator_settings.c_thrld;//float(fn["neg_c_min"].real());
    learning_settings.neg_c_max = float(fn["neg_c_max"].real());
    learning_settings.pos_c_min = float(fn["pos_c_min"].real());
    learning_settings.pos_c_max = integrator_settings.c_thrld + 0.1f;//float(fn["pos_c_max"].real());
    learning_settings.overlap_thrld = float(fn["overlap_thrld"].real());
    learning_settings.learn_scales[0] = float(fn["learn_scales_0"].real());
    learning_settings.learn_scales[1] = float(fn["learn_scales_1"].real());
    learning_settings.learn_scales[2] = float(fn["learn_scales_2"].real());
    learning_settings.learn_angles[0] = float(fn["learn_angles_0"].real());
    learning_settings.learn_angles[1] = float(fn["learn_angles_1"].real());
    learning_settings.learn_angles[2] = float(fn["learn_angles_2"].real());
}

void ETLDParams::write_model_settings(FileStorage & fs) const
{
    fs << "model_settings" << "{";
    fs.write("ex_w", model_settings.ex_w);
    fs.write("ex_h", model_settings.ex_h);
    fs.write("ex_n", model_settings.ex_n);
    fs.write("ex_const_n", model_settings.ex_const_n);
    fs << "}";
}
void ETLDParams::write_classifier_settings(FileStorage & fs) const
{
    fs << "classifier_settings" << "{";
    fs.write("fern_n", classifier_settings.fern_n);
    fs.write("fern_k", classifier_settings.fern_k);
    fs << "}";
}
void ETLDParams::write_detector_settings(FileStorage & fs) const
{
    fs << "detector_settings" << "{";
    fs.write("grid_step", detector_settings.grid_step);
    fs.write("r_thrld", detector_settings.r_thrld);
    fs.write("max_candidates_num", detector_settings.max_candidates_num);
    fs.write("detect_scales_0", detector_settings.detect_scales[0]);
    fs.write("detect_scales_1", detector_settings.detect_scales[1]);
    fs.write("detect_scales_2", detector_settings.detect_scales[2]);
    fs.write("min_scale", detector_settings.min_scale);
    fs.write("max_scale", detector_settings.max_scale);
    fs << "}";
}
void ETLDParams::write_tracker_settings(FileStorage & fs) const
{
    fs << "tracker_settings" << "{";
    fs.write("pyramid_levels", tracker_settings.pyramid_levels);
    fs.write("win_sz", tracker_settings.win_sz);
    fs.write("npts_width", tracker_settings.npts_width);
    fs.write("npts_height", tracker_settings.npts_height);
    fs.write("nmax_iter", tracker_settings.nmax_iter);
    fs.write("min_delta", tracker_settings.min_delta);
    fs.write("variance_thrld_div", tracker_settings.variance_thrld_div);
    fs.write("subgrid_sz", tracker_settings.subgrid_sz);
    fs.write("max_fb_err_coeff", tracker_settings.max_fb_err_coeff);
    fs.write("min_part_for_scale", tracker_settings.min_part_for_scale);
    fs.write("use_fast", tracker_settings.use_fast);
    fs << "}";
}
void ETLDParams::write_integrator_settings(FileStorage & fs) const
{
    fs << "integrator_settings" << "{";
    fs.write("c_thrld", integrator_settings.c_thrld);
    fs.write("overlap_thrld", integrator_settings.overlap_thrld);
    fs.write("frames_valid", integrator_settings.frames_valid);
    fs.write("a_xy", integrator_settings.a_xy);
    fs.write("a_wh", integrator_settings.a_wh);
    fs << "}";
}
void ETLDParams::write_learning_settings(FileStorage & fs) const
{
    fs << "learning_settings" << "{";
    fs.write("reaim_en", learning_settings.reaim_en);
    fs.write("reaim_D_thrld_div", learning_settings.reaim_D_thrld_div);
    fs.write("fast_update", learning_settings.fast_update);
    fs.write("neg_r_min", learning_settings.neg_r_min);
    fs.write("neg_r_max", learning_settings.neg_r_max);
    fs.write("pos_r_min", learning_settings.pos_r_min);
    fs.write("pos_r_max", learning_settings.pos_r_max);
    fs.write("neg_c_min", learning_settings.neg_c_min);
    fs.write("neg_c_max", learning_settings.neg_c_max);
    fs.write("pos_c_min", learning_settings.pos_c_min);
    fs.write("pos_c_max", learning_settings.pos_c_max);
    fs.write("overlap_thrld", learning_settings.overlap_thrld);
    fs.write("learn_scales_0", learning_settings.learn_scales[0]);
    fs.write("learn_scales_1", learning_settings.learn_scales[1]);
    fs.write("learn_scales_2", learning_settings.learn_scales[2]);
    fs.write("learn_angles_0", learning_settings.learn_angles[0]);
    fs.write("learn_angles_1", learning_settings.learn_angles[1]);
    fs.write("learn_angles_2", learning_settings.learn_angles[2]);
    fs << "}";
}

}
}
