#ifndef ETLDINTEGRATOR_H
#define ETLDINTEGRATOR_H

#include "etld/etld_global.h"
#include "etld/etldparams.h"

namespace cv
{
namespace etld
{
class EtldClassifier;
class EtldModel;
class EtldIntegrator
{
public:
    EtldIntegrator();
    ~EtldIntegrator();

    void init(const cv::Mat_<uint8_t> & frame, const etld_object & object, const EtldClassifier & classifier, const EtldModel & model, const ETLDParams & params);

    void integrate(etld_object & object, const etld_tracker_candidate & tracker_candidate, const etld_detector_candidate * detector_candidates, const int & detector_candidates_num);
private:
    void clusterise(const etld_detector_candidate * detector_candidates, const int & detector_candidates_num);
    float overlap(const cv::Rect2i & p0, const cv::Rect2i & p1);
private:
    int clusters_n;
    etld_cluster * clusters;
    int * indexes;
    float * weights;
    float c_thrld;
    float overlap_thrld;
    int frames_valid;
    float a_xy;
    float a_wh;
};
}
}

#endif // ETLDINTEGRATOR_H
