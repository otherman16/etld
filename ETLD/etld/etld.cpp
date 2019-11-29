#include "etld.h"
#include <cstring>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
using namespace std;
#include "etld/my_fast.h"

namespace cv
{
namespace etld
{
ETLD::ETLD()
{
    _on = false;

    roi = cv::Rect2i(0, 0, ETLD_W, ETLD_H);

    object.valid = false;
    object.tracker_reinit =  false;
    object.update = false;
    tracker_candidate.success = false;
    detector_candidates_num = 0;
    detector_candidates = nullptr;

    _etld_time = 0;
    _frame_time = 0;
    _init_time = 0;
    _detector_time = 0;
    _tracker_time = 0;
    _integrator_time = 0;
    _update_time = 0;
}
ETLD::ETLD(const ETLDParams & params):
    params(params)
{
    _on = false;

    roi = cv::Rect2i(0, 0, ETLD_W, ETLD_H);

    object.valid = false;
    object.tracker_reinit =  false;
    object.update = false;
    tracker_candidate.success = false;
    detector_candidates_num = 0;
    detector_candidates = nullptr;

    _etld_time = 0;
    _frame_time = 0;
    _init_time = 0;
    _detector_time = 0;
    _tracker_time = 0;
    _integrator_time = 0;
    _update_time = 0;
}
bool ETLD::init(const Mat & image, const Rect2d & target)
{
    if(target.x < 10 || target.y < 10 || target.x + target.width >= image.cols-10 || target.y + target.height >= image.rows-10 || target.width < 5 || target.height < 5)
    {
        deinit();
        return false;
    }

    chrono::steady_clock::time_point begin_init = chrono::steady_clock::now();

    _on = true;

    detector_candidates_num = 0;
    if(detector_candidates) delete [] detector_candidates;
    detector_candidates = new etld_detector_candidate[params.detector_settings.max_candidates_num];

    object.valid = true;
    object.tracker_reinit = true;
    object.update = false;
    object.C = 1;
    object.R = 1;

    cv::Rect2i new_object = target;

    if(params.learning_settings.reaim_en) reaim(image, new_object);

    object.window = new_object;

    frame.init(object.window, image, roi);

    cv::Mat_<uint8_t> subframe = frame.get_subframe();
    cv::Mat_<uint8_t> smoothed_subframe = frame.get_smoothed_subframe();

    learning.init(  smoothed_subframe, object, classifier, model, params);
    detector.init(  smoothed_subframe, object, classifier, model, params);
    tracker.init(   subframe,          object, classifier, model, params);
    integrator.init(smoothed_subframe, object, classifier, model, params);

    _init_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_init)).count());

    return true;
}
bool ETLD::update(const Mat & image, Rect2d & target)
{
    if(_on)
    {
        chrono::steady_clock::time_point begin_etld = chrono::steady_clock::now();

        chrono::steady_clock::time_point begin_frame = chrono::steady_clock::now();

        if(object.valid) frame.next(object.window, tracker_candidate.window, image, roi);
        else frame.find(object.window, tracker_candidate.window, image, roi);

        cv::Mat_<uint8_t> prev_subframe = frame.get_prev_subframe();
        cv::Mat_<uint8_t> subframe = frame.get_subframe();
        cv::Mat_<uint8_t> smoothed_subframe = frame.get_smoothed_subframe();

        _frame_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_frame)).count());

        chrono::steady_clock::time_point begin_detector = chrono::steady_clock::now();
        detector_candidates_num = detector.detect(smoothed_subframe, object, detector_candidates, classifier, model);
        _detector_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_detector)).count());

        chrono::steady_clock::time_point begin_tracker = chrono::steady_clock::now();
        tracker.track(prev_subframe, subframe, smoothed_subframe, object, tracker_candidate, classifier, model);
        _tracker_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_tracker)).count());

        chrono::steady_clock::time_point begin_integrator = chrono::steady_clock::now();
        integrator.integrate(object, tracker_candidate, detector_candidates, detector_candidates_num);
        _integrator_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_integrator)).count());

        if(object.update)
        {
            chrono::steady_clock::time_point begin_update = chrono::steady_clock::now();
            learning.update(smoothed_subframe, object, classifier, model);
            _update_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_update)).count());
        }

        _etld_time = int((chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - begin_etld)).count());

        etld_object object;
        get_object(&object);
        target = object.window;
        return object.valid;
    }
    return false;
}
void ETLD::deinit()
{
    _on = false;

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
void ETLD::reaim(const cv::Mat & f, cv::Rect2i & aim)
{
    EtldImage aim_img(f, aim);
    int thrld = aim_img.D() / params.learning_settings.reaim_D_thrld_div;
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
ETLD::~ETLD()
{
    deinit();
}

}
}
