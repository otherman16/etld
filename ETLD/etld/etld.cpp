#include "etld.h"
#include <cstring>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
using namespace std;
#include "etld/my_fast.h"

#include <opencv2/tracking.hpp>
namespace cv
{

ETLD::ETLD()
{
    _on = false;
    _init = false;
    _deinit = false;

    roi = cv::Rect_<int>(0, 0, ETLD_W, ETLD_H);
    new_object = cv::Rect_<int>(0, 0, 10, 10);

    object.valid = false;
    object.tracker_reinit =  false;
    object.update = false;
    tracker_candidate.success = false;
    detector_candidates_num = 0;
    detector_candidates = nullptr;

    settings.model_settings.ex_w = 15;
    settings.model_settings.ex_h = 15;
    settings.model_settings.ex_n = 100;
    settings.model_settings.ex_const_n = 50;

    settings.classifier_settings.fern_n = 20;
    settings.classifier_settings.fern_k = 10;

    settings.detector_settings.grid_step = 4;
    settings.detector_settings.r_thrld = 0.7f;
    settings.detector_settings.max_candidates_num = 8;
    settings.detector_settings.detect_scales[0] = 1.00f;
    settings.detector_settings.detect_scales[1] = 0.75f;
    settings.detector_settings.detect_scales[2] = 1.25f;
    settings.detector_settings.min_scale        = 0.80f;
    settings.detector_settings.max_scale        = 1.20f;

    settings.tracker_settings.pyramid_levels        = 3;
    settings.tracker_settings.win_sz                = 5;
    settings.tracker_settings.npts_width            = 10;
    settings.tracker_settings.npts_height           = 10;
    settings.tracker_settings.nmax_iter             = 20;
    settings.tracker_settings.min_delta             = 0.03f;
    settings.tracker_settings.variance_thrld_div    = 20;
    settings.tracker_settings.subgrid_sz            = 7;
    settings.tracker_settings.max_fb_err_coeff      = 0.05f;
    settings.tracker_settings.min_part_for_scale    = 0.1f;
    settings.tracker_settings.use_fast              = false;

    settings.integrator_settings.c_thrld = 0.6f;
    settings.integrator_settings.overlap_thrld = 0.5;
    settings.integrator_settings.frames_valid = 40;
    settings.integrator_settings.a_xy = 0.9f;
    settings.integrator_settings.a_wh = 0.7f;

    settings.learning_settings.reaim_en = true;
    settings.learning_settings.reaim_D_thrld_div = 400;
    settings.learning_settings.fast_update       = false;
    settings.learning_settings.neg_r_min         = 0.1f;
    settings.learning_settings.neg_r_max         = settings.detector_settings.r_thrld + 0.1f;//0.6f;
    settings.learning_settings.pos_r_min         = settings.detector_settings.r_thrld - 0.1f;//0.6f;
    settings.learning_settings.pos_r_max         = 0.9f;
    settings.learning_settings.neg_c_min         = settings.integrator_settings.c_thrld;//0.7f;
    settings.learning_settings.neg_c_max         = 0.9f;
    settings.learning_settings.pos_c_min         = 0.1f;
    settings.learning_settings.pos_c_max         = settings.integrator_settings.c_thrld + 0.1f;//0.7f;
    settings.learning_settings.overlap_thrld     = 0.5f;
    settings.learning_settings.learn_scales[0]   = 1.00f;
    settings.learning_settings.learn_scales[1]   = 0.75f;
    settings.learning_settings.learn_scales[2]   = 1.25f;
    settings.learning_settings.learn_angles[0]   = 0.00f;
    settings.learning_settings.learn_angles[1]   = -0.25f;
    settings.learning_settings.learn_angles[2]   = 0.34f;

    etld_time = 0;
    frame_time = 0;
    init_time = 0;
    detector_time = 0;
    tracker_time = 0;
    integrator_time = 0;
    update_time = 0;
}
bool ETLD::load_settings(string fileName)
{
    ifstream config;
    config.open(fileName, ios::in);
    printf("ETLD::load_settings: reading config file %s ...\n", fileName.c_str());
    if(config.is_open())
    {
        string line;
        while(getline(config, line))
        {
            size_t pos = line.find("=");
            string key = line.substr(0, pos);
            string val = line.substr(pos + 1, line.size());
            if(!key.compare("model_settings.ex_w")) settings.model_settings.ex_w = stoi(val);
            else if(!key.compare("model_settings.ex_h")) settings.model_settings.ex_h = stoi(val);
            else if(!key.compare("model_settings.ex_n")) settings.model_settings.ex_n = stoi(val);
            else if(!key.compare("model_settings.ex_const_n")) settings.model_settings.ex_const_n = stoi(val);
            else if(!key.compare("classifier_settings.fern_n")) settings.classifier_settings.fern_n = stoi(val);
            else if(!key.compare("classifier_settings.fern_k")) settings.classifier_settings.fern_k = stoi(val);
            else if(!key.compare("detector_settings.grid_step")) settings.detector_settings.grid_step = stoi(val);
            else if(!key.compare("detector_settings.r_thrld")) settings.detector_settings.r_thrld = (stof(val));
            else if(!key.compare("detector_settings.max_candidates_num")) settings.detector_settings.max_candidates_num = stoi(val);
            else if(!key.compare("detector_settings.detect_scales[0]")) settings.detector_settings.detect_scales[0] = (stof(val));
            else if(!key.compare("detector_settings.detect_scales[1]")) settings.detector_settings.detect_scales[1] = (stof(val));
            else if(!key.compare("detector_settings.detect_scales[2]")) settings.detector_settings.detect_scales[2] = (stof(val));
            else if(!key.compare("detector_settings.min_scale")) settings.detector_settings.min_scale = (stof(val));
            else if(!key.compare("detector_settings.max_scale")) settings.detector_settings.max_scale = (stof(val));
            else if(!key.compare("tracker_settings.pyramid_levels")) settings.tracker_settings.pyramid_levels = stoi(val);
            else if(!key.compare("tracker_settings.win_sz")) settings.tracker_settings.win_sz = stoi(val);
            else if(!key.compare("tracker_settings.npts_width")) settings.tracker_settings.npts_width = stoi(val);
            else if(!key.compare("tracker_settings.npts_height")) settings.tracker_settings.npts_height = stoi(val);
            else if(!key.compare("tracker_settings.nmax_iter")) settings.tracker_settings.nmax_iter = stoi(val);
            else if(!key.compare("tracker_settings.min_delta")) settings.tracker_settings.min_delta = stof(val);
            else if(!key.compare("tracker_settings.variance_thrld_div")) settings.tracker_settings.variance_thrld_div = stoi(val);
            else if(!key.compare("tracker_settings.subgrid_sz")) settings.tracker_settings.subgrid_sz = stoi(val);
            else if(!key.compare("tracker_settings.max_fb_err_coeff")) settings.tracker_settings.max_fb_err_coeff = (stof(val));
            else if(!key.compare("tracker_settings.min_part_for_scale")) settings.tracker_settings.min_part_for_scale = (stof(val));
            else if(!key.compare("tracker_settings.use_fast")) settings.tracker_settings.use_fast = (stoi(val));
            else if(!key.compare("integrator_settings.c_thrld")) settings.integrator_settings.c_thrld = (stof(val));
            else if(!key.compare("integrator_settings.overlap_thrld")) settings.integrator_settings.overlap_thrld = (stof(val));
            else if(!key.compare("integrator_settings.frames_(stoi(val))id")) settings.integrator_settings.frames_valid = stoi(val);
            else if(!key.compare("integrator_settings.a_xy")) settings.integrator_settings.a_xy = (stof(val));
            else if(!key.compare("integrator_settings.a_wh")) settings.integrator_settings.a_wh = (stof(val));
            else if(!key.compare("learning_settings.reaim_en")) settings.learning_settings.reaim_en = bool(stoi(val));
            else if(!key.compare("learning_settings.reaim_D_thrld_div")) settings.learning_settings.reaim_D_thrld_div = stoi(val);
            else if(!key.compare("learning_settings.fast_update")) settings.learning_settings.fast_update = bool(stoi(val));
            else if(!key.compare("learning_settings.neg_r_min")) settings.learning_settings.neg_r_min = (stof(val));
            else if(!key.compare("learning_settings.neg_r_max")) settings.learning_settings.neg_r_max = (stof(val));
            else if(!key.compare("learning_settings.pos_r_min")) settings.learning_settings.pos_r_min = (stof(val));
            else if(!key.compare("learning_settings.pos_r_max")) settings.learning_settings.pos_r_max = (stof(val));
            else if(!key.compare("learning_settings.neg_c_min")) settings.learning_settings.neg_c_min = (stof(val));
            else if(!key.compare("learning_settings.neg_c_max")) settings.learning_settings.neg_c_max = (stof(val));
            else if(!key.compare("learning_settings.pos_c_min")) settings.learning_settings.pos_c_min = (stof(val));
            else if(!key.compare("learning_settings.pos_c_max")) settings.learning_settings.pos_c_max = (stof(val));
            else if(!key.compare("learning_settings.overlap_thrld")) settings.learning_settings.overlap_thrld = (stof(val));
            else if(!key.compare("learning_settings.learn_scales[0]")) settings.learning_settings.learn_scales[0] = (stof(val));
            else if(!key.compare("learning_settings.learn_scales[1]")) settings.learning_settings.learn_scales[1] = (stof(val));
            else if(!key.compare("learning_settings.learn_scales[2]")) settings.learning_settings.learn_scales[2] = (stof(val));
            else if(!key.compare("learning_settings.learn_angles[0]")) settings.learning_settings.learn_angles[0] = (stof(val));
            else if(!key.compare("learning_settings.learn_angles[1]")) settings.learning_settings.learn_angles[1] = (stof(val));
            else if(!key.compare("learning_settings.learn_angles[2]")) settings.learning_settings.learn_angles[2] = (stof(val));
        }
        config.close();
        printf("\tDone\n");
    }
    else
    {
        printf("\tError\n");
        printf("\tDefault settings were saved\n");
        return false;
    }
    return true;
}
string ETLD::str_settings()
{
    ostringstream os;
    os << "\t{model_settings.ex_w : " << settings.model_settings.ex_w << "}" << endl;
    os << "\t{model_settings.ex_h : " << settings.model_settings.ex_h << "}" << endl;
    os << "\t{model_settings.ex_n : " << settings.model_settings.ex_n << "}" << endl;
    os << "\t{model_settings.ex_const_n : " << settings.model_settings.ex_const_n << "}" << endl;
    os << "\t{classifier_settings.fern_n : " << settings.classifier_settings.fern_n << "}" << endl;
    os << "\t{classifier_settings.fern_k : " << settings.classifier_settings.fern_k << "}" << endl;
    os << "\t{detector_settings.grid_step : " << settings.detector_settings.grid_step << "}" << endl;
    os << "\t{detector_settings.r_thrld : " << settings.detector_settings.r_thrld << "}" << endl;
    os << "\t{detector_settings.max_candidates_num : " << settings.detector_settings.max_candidates_num << "}" << endl;
    os << "\t{detector_settings.detect_scales[0] : " << settings.detector_settings.detect_scales[0] << "}" << endl;
    os << "\t{detector_settings.detect_scales[1] : " << settings.detector_settings.detect_scales[1] << "}" << endl;
    os << "\t{detector_settings.detect_scales[2] : " << settings.detector_settings.detect_scales[2] << "}" << endl;
    os << "\t{detector_settings.min_scale : " << settings.detector_settings.min_scale << "}" << endl;
    os << "\t{detector_settings.max_scale : " << settings.detector_settings.max_scale << "}" << endl;
    os << "\t{tracker_settings.pyramid_levels : " << settings.tracker_settings.pyramid_levels << "}" << endl;
    os << "\t{tracker_settings.win_sz : " << settings.tracker_settings.win_sz << "}" << endl;
    os << "\t{tracker_settings.npts_width : " << settings.tracker_settings.npts_width << "}" << endl;
    os << "\t{tracker_settings.npts_height : " << settings.tracker_settings.npts_height << "}" << endl;
    os << "\t{tracker_settings.nmax_iter : " << settings.tracker_settings.nmax_iter << "}" << endl;
    os << "\t{tracker_settings.min_delta : " << settings.tracker_settings.min_delta << "}" << endl;
    os << "\t{tracker_settings.variance_thrld_div : " << settings.tracker_settings.variance_thrld_div << "}" << endl;
    os << "\t{tracker_settings.subgrid_sz : " << settings.tracker_settings.subgrid_sz << "}" << endl;
    os << "\t{tracker_settings.max_fb_err_coeff : " << settings.tracker_settings.max_fb_err_coeff << "}" << endl;
    os << "\t{tracker_settings.min_part_for_scale : " << settings.tracker_settings.min_part_for_scale << "}" << endl;
    os << "\t{tracker_settings.use_fast : " << settings.tracker_settings.use_fast << "}" << endl;
    os << "\t{integrator_settings.c_thrld : " << settings.integrator_settings.c_thrld << "}" << endl;
    os << "\t{integrator_settings.overlap_thrld : " << settings.integrator_settings.overlap_thrld << "}" << endl;
    os << "\t{integrator_settings.frames_valid : " << settings.integrator_settings.frames_valid << "}" << endl;
    os << "\t{integrator_settings.a_xy : " << settings.integrator_settings.a_xy << "}" << endl;
    os << "\t{integrator_settings.a_wh : " << settings.integrator_settings.a_wh << "}" << endl;
    os << "\t{learning_settings.reaim_en : " << settings.learning_settings.reaim_en << "}" << endl;
    os << "\t{learning_settings.reaim_D_thrld_div : " << settings.learning_settings.reaim_D_thrld_div << "}" << endl;
    os << "\t{learning_settings.fast_update : " << settings.learning_settings.fast_update << "}" << endl;
    os << "\t{learning_settings.neg_r_min : " << settings.learning_settings.neg_r_min << "}" << endl;
    os << "\t{learning_settings.neg_r_max : " << settings.learning_settings.neg_r_max << "}" << endl;
    os << "\t{learning_settings.pos_r_min : " << settings.learning_settings.pos_r_min << "}" << endl;
    os << "\t{learning_settings.pos_r_max : " << settings.learning_settings.pos_r_max << "}" << endl;
    os << "\t{learning_settings.neg_c_min : " << settings.learning_settings.neg_c_min << "}" << endl;
    os << "\t{learning_settings.neg_c_max : " << settings.learning_settings.neg_c_max << "}" << endl;
    os << "\t{learning_settings.pos_c_min : " << settings.learning_settings.pos_c_min << "}" << endl;
    os << "\t{learning_settings.pos_c_max : " << settings.learning_settings.pos_c_max << "}" << endl;
    os << "\t{learning_settings.overlap_thrld : " << settings.learning_settings.overlap_thrld << "}" << endl;
    os << "\t{learning_settings.learn_scales[0] : " << settings.learning_settings.learn_scales[0] << "}" << endl;
    os << "\t{learning_settings.learn_scales[1] : " << settings.learning_settings.learn_scales[1] << "}" << endl;
    os << "\t{learning_settings.learn_scales[2] : " << settings.learning_settings.learn_scales[2] << "}" << endl;
    os << "\t{learning_settings.learn_angles[0] : " << settings.learning_settings.learn_angles[0] << "}" << endl;
    os << "\t{learning_settings.learn_angles[1] : " << settings.learning_settings.learn_angles[1] << "}" << endl;
    os << "\t{learning_settings.learn_angles[2] : " << settings.learning_settings.learn_angles[2] << "}" << endl;
    return os.str();
}
string ETLD::str_timings()
{
    ostringstream os;
    os << "\t{etld_time : " << etld_time << "}" << endl;
    os << "\t\t{frame_time : " << frame_time << "}" << endl;
    os << "\t\t{init_time : " << init_time << "}" << endl;
    os << "\t\t{detector_time : " << detector_time << "}" << endl;
    os << "\t\t{tracker_time : " << tracker_time << "}" << endl;
    os << "\t\t{integrator_time : " << integrator_time << "}" << endl;
    os << "\t\t{update_time : " << update_time << "}" << endl;
    return os.str();
}
int ETLD::get_etld_time()
{
    return etld_time;
}
int ETLD::get_frame_time()
{
    return frame_time;
}
int ETLD::get_init_time()
{
    return init_time;
}
int ETLD::get_detector_time()
{
    return detector_time;
}
int ETLD::get_tracker_time()
{
    return tracker_time;
}
int ETLD::get_integrator_time()
{
    return integrator_time;
}
int ETLD::get_update_time()
{
    return update_time;
}
void ETLD::init(const int & x, const int & y, const int & w, const int & h, etld_settings * settings)
{
    init(cv::Rect_<int>(x, y, w, h), settings);
}
void ETLD::init(const cv::Rect_<int> & object, etld_settings * settings)
{
    _init = true;
    _deinit = false;
    new_object = object;

    if(settings)
    {
        memcpy(&this->settings, settings, sizeof(etld_settings));
    }
}
void ETLD::deinit()
{
    _init = false;
    _deinit = true;
}
bool ETLD::isOn()
{
    return (_on || _init || _deinit);
}
void ETLD::get_object(etld_object * object)
{
    float x_ratio = float(roi.width) / float(ETLD_W);
    float y_ratio = float(roi.height) / float(ETLD_H);
    memcpy(object, &(this->object), sizeof(etld_object));
    object->window.x = int(roundf(float(object->window.x) * x_ratio)) + roi.x;
    object->window.y = int(roundf(float(object->window.y) * y_ratio)) + roi.y;
    object->window.width  = int(roundf(float(object->window.width ) * x_ratio));
    object->window.height  = int(roundf(float(object->window.height ) * y_ratio));
}
void ETLD::get_tracker_candidates(etld_tracker_candidate * tracker_candidate)
{
    float x_ratio = float(roi.width) / float(ETLD_W);
    float y_ratio = float(roi.height) / float(ETLD_H);
    memcpy(tracker_candidate, &(this->tracker_candidate), sizeof(etld_tracker_candidate));
    tracker_candidate->window.x = int(roundf(float(tracker_candidate->window.x) * x_ratio)) + roi.x;
    tracker_candidate->window.y = int(roundf(float(tracker_candidate->window.y) * y_ratio)) + roi.y;
    tracker_candidate->window.width  = int(roundf(float(tracker_candidate->window.width ) * x_ratio));
    tracker_candidate->window.height  = int(roundf(float(tracker_candidate->window.height ) * y_ratio));
}
int ETLD::get_detector_candidates(etld_detector_candidate * detector_candidates)
{
    float x_ratio = float(roi.width) / float(ETLD_W);
    float y_ratio = float(roi.height) / float(ETLD_H);
    for(int i = 0; i < detector_candidates_num; ++i)
    {
        memcpy(&detector_candidates[i], &(this->detector_candidates[i]), sizeof(etld_detector_candidate));
        detector_candidates[i].window.x = int(roundf(float(detector_candidates[i].window.x) * x_ratio)) + roi.x;
        detector_candidates[i].window.y = int(roundf(float(detector_candidates[i].window.y) * y_ratio)) + roi.y;
        detector_candidates[i].window.width  = int(roundf(float(detector_candidates[i].window.width ) * x_ratio));
        detector_candidates[i].window.height  = int(roundf(float(detector_candidates[i].window.height ) * y_ratio));
    }
    return detector_candidates_num;
}
void ETLD::reaim(const cv::Mat_<uint8_t> & f, cv::Rect_<int> & aim)
{
    EtldImage aim_img(f, aim);
    int thrld = aim_img.D() / settings.learning_settings.reaim_D_thrld_div;
    int ret_num_corners = int(aim.width) * int(aim.height);
    fast_xy * sp = new fast_xy[size_t(ret_num_corners)];
    fast_byte * im = reinterpret_cast<fast_byte *>(f.data) + uint32_t(aim.x) + uint32_t(aim.y) * uint32_t(f.cols);
    my_fast_detect(im, int(aim.width), int(aim.height), int(f.cols), int(thrld), sp, ret_num_corners, &ret_num_corners);
    if(ret_num_corners > 0)
    {
        int x_min = sp[0].x;
        int x_max = sp[0].x;
        int y_min = sp[0].y;
        int y_max = sp[0].y;
        for(int i = 1; i < ret_num_corners; ++i)
        {
            if(sp[i].x < x_min) x_min = sp[i].x;
            else if(sp[i].x > x_max) x_max = sp[i].x;
            if(sp[i].y < y_min) y_min = sp[i].y;
            else if(sp[i].y > y_max) y_max = sp[i].y;
        }
        int w = x_max - x_min;
        if(w < 5) w = 5;
        int h = y_max - y_min;
        if(h < 5) h = 5;
        aim.x = aim.x + x_min;
        aim.y = aim.y + y_min;
        aim.width = w;
        aim.height = h;
    }
    delete [] sp;
}
void ETLD::new_frame(uint8_t * f, const uint32_t & fw, const uint32_t & fh, cv::Rect_<int> & roi)
{
    new_frame(cv::Mat_<uint8_t>(int(fh), int(fw), f, cv::Mat::AUTO_STEP), roi);
}
void ETLD::new_frame(const cv::Mat_<uint8_t> & f, cv::Rect_<int> & roi)
{
    if(_init)
    {
        chrono::steady_clock::time_point begin_init = chrono::steady_clock::now();

        _on = true;
        _init = false;
        _deinit = false;

        detector_candidates_num = 0;
        if(detector_candidates) delete [] detector_candidates;
        detector_candidates = new etld_detector_candidate[settings.detector_settings.max_candidates_num];

        object.valid = true;
        object.tracker_reinit = true;
        object.update = false;
        object.C = 1;
        object.R = 1;

        if(settings.learning_settings.reaim_en) reaim(f, new_object);

        object.window.x = new_object.x;
        object.window.y = new_object.y;
        object.window.width = new_object.width;
        object.window.height = new_object.height;

        frame.init(object.window, f, this->roi);

        roi = this->roi;

        cv::Mat_<uint8_t> subframe = frame.get_subframe();
        cv::Mat_<uint8_t> smoothed_subframe = frame.get_smoothed_subframe();

        learning.init(  smoothed_subframe, object, classifier, model, settings);
        detector.init(  smoothed_subframe, object, classifier, model, settings);
        tracker.init(   subframe,          object, classifier, model, settings);
        integrator.init(smoothed_subframe, object, classifier, model, settings);

        init_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_init)).count());
    }
    else if(_deinit)
    {
        _on = false;
        _init = false;
        _deinit = false;

        object.valid = false;
        object.tracker_reinit = false;
        object.update = false;
        object.C = 0;
        object.R = 0;
        object.window.x = 0;
        object.window.y = 0;
        object.window.width = 0;
        object.window.height = 0;
    }
    else if(_on)
    {
        chrono::steady_clock::time_point begin_etld = chrono::steady_clock::now();

        chrono::steady_clock::time_point begin_frame = chrono::steady_clock::now();

        if(object.valid) frame.next(object.window, tracker_candidate.window, f, this->roi);
        else frame.find(object.window, tracker_candidate.window, f, this->roi);
        roi = this->roi;
        cv::Mat_<uint8_t> prev_subframe = frame.get_prev_subframe();
        cv::Mat_<uint8_t> subframe = frame.get_subframe();
        cv::Mat_<uint8_t> smoothed_subframe = frame.get_smoothed_subframe();

        frame_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_frame)).count());

        chrono::steady_clock::time_point begin_detector = chrono::steady_clock::now();
        detector_candidates_num = detector.detect(smoothed_subframe, object, detector_candidates, classifier, model);
        detector_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_detector)).count());

        chrono::steady_clock::time_point begin_tracker = chrono::steady_clock::now();
        tracker.track(prev_subframe, subframe, smoothed_subframe, object, tracker_candidate, classifier, model);
        tracker_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_tracker)).count());

        chrono::steady_clock::time_point begin_integrator = chrono::steady_clock::now();
        integrator.integrate(object, tracker_candidate, detector_candidates, detector_candidates_num);
        integrator_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_integrator)).count());

        if(object.update)
        {
            chrono::steady_clock::time_point begin_update = chrono::steady_clock::now();
            learning.update(smoothed_subframe, object, classifier, model);
            update_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_update)).count());
        }

        etld_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_etld)).count());
    }
}
ETLD::~ETLD()
{
}

}
