#ifndef ETLD_CLASS_H
#define ETLD_CLASS_H

#include <opencv2/opencv.hpp>

#include "etld/etld_global.h"

#include "etld/etldclassifier.h"
#include "etld/etlddetector.h"
#include "etld/etldimage.h"
#include "etld/etldintegrator.h"
#include "etld/etldlearning.h"
#include "etld/etldmodel.h"
#include "etld/etldtracker.h"
#include "etld/etldframe.h"
#include "etld/etldparams.h"

namespace cv
{
namespace etld
{
class ETLD : public Algorithm
{
public:
    ETLD();
    ETLD(const ETLDParams & params);
    ~ETLD();

    inline int etld_time() const {return _etld_time;}
    inline int frame_time() const {return _frame_time;}
    inline int init_time() const {return _init_time;}
    inline int detector_time() const {return _detector_time;}
    inline int tracker_time() const {return _tracker_time;}
    inline int integrator_time() const {return _integrator_time;}
    inline int update_time() const {return _update_time;}

    bool init(const Mat & image, const Rect2d & target);
    bool update(const Mat & image, Rect2d & target );
    void deinit();
    inline bool on() const {return _on;}
    void get_object(etld_object * object);
    void get_tracker_candidates(etld_tracker_candidate * tracker_candidate);
    int get_detector_candidates(etld_detector_candidate * detector_candidates);

    void read( const FileNode& fn ) {params.read(fn);}
    void write( FileStorage& fs ) const {params.write(fs);}

    inline ETLDParams * ptr_params() {return &params;}

    static Ptr<ETLD> create() {return makePtr<ETLD>();}
    static Ptr<ETLD> create(const ETLDParams & params) {return makePtr<ETLD>(params);}

private:
    void reaim(const cv::Mat & f, cv::Rect2i & aim);

private:
    volatile bool _on;
    cv::Rect2i roi;

    etld_object object;
    etld_tracker_candidate tracker_candidate;
    int detector_candidates_num;
    etld_detector_candidate * detector_candidates;

    ETLDParams params;

    EtldFrame frame;
    EtldModel model;
    EtldClassifier classifier;
    EtldDetector detector;
    EtldTracker tracker;
    EtldIntegrator integrator;
    EtldLearning learning;

    volatile int _etld_time;
    volatile int _frame_time;
    volatile int _init_time;
    volatile int _detector_time;
    volatile int _tracker_time;
    volatile int _integrator_time;
    volatile int _update_time;
};
}
}

#endif // ETLD_CLASS_H
