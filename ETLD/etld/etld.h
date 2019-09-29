#ifndef ETLD_CLASS_H
#define ETLD_CLASS_H

#include "etld/etld_global.h"

#include "etld/etldclassifier.h"
#include "etld/etlddetector.h"
#include "etld/etldimage.h"
#include "etld/etldintegrator.h"
#include "etld/etldlearning.h"
#include "etld/etldmodel.h"
#include "etld/etldtracker.h"
#include "etld/etldframe.h"

using namespace etld;
class ETLD
{
public:
    ETLD();
    ~ETLD();

    bool load_settings(std::string);
    std::string str_settings();
    std::string str_timings();

    int get_etld_time();
    int get_frame_time();
    int get_init_time();
    int get_detector_time();
    int get_tracker_time();
    int get_integrator_time();
    int get_update_time();

    void init(const int & x, const int & y, const int & w, const int & h, etld_settings * settings=nullptr);
    void init(const cv::Rect_<int> & object, etld_settings * settings=nullptr);
    void deinit();
    bool isOn();
    void get_object(etld_object * object);
    void get_tracker_candidates(etld_tracker_candidate * tracker_candidate);
    int get_detector_candidates(etld_detector_candidate * detector_candidates);

    void new_frame(uint8_t * f, const uint32_t & fw, const uint32_t & fh, cv::Rect_<int> & roi);
    void new_frame(const cv::Mat_<uint8_t> & f, cv::Rect_<int> & roi);

private:
    void reaim(const cv::Mat_<uint8_t> & f, cv::Rect_<int> & aim);

private:
    volatile bool _on;
    volatile bool _init;
    volatile bool _deinit;
    cv::Rect_<int> roi;
    cv::Rect_<int> new_object;

    etld_object object;
    etld_tracker_candidate tracker_candidate;
    int detector_candidates_num;
    etld_detector_candidate * detector_candidates;

    EtldFrame frame;
    EtldModel model;
    EtldClassifier classifier;
    EtldDetector detector;
    EtldTracker tracker;
    EtldIntegrator integrator;
    EtldLearning learning;

    etld_settings settings;

    volatile int etld_time;
    volatile int frame_time;
    volatile int init_time;
    volatile int detector_time;
    volatile int tracker_time;
    volatile int integrator_time;
    volatile int update_time;

    cv::Mat_<uint8_t> * smoothed_subframe_copy;
    EtldModel model_copy;
    EtldClassifier classifier_copy;
};

#endif // ETLD_CLASS_H
