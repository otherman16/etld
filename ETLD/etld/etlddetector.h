#ifndef ETLDDETECTOR_H
#define ETLDDETECTOR_H

#include "etld/etld_global.h"
#include "etld/etldparams.h"

namespace cv
{
namespace etld
{
class EtldClassifier;
class EtldModel;
class EtldDetector
{
public:
    EtldDetector();
    ~EtldDetector();

    void init(const cv::Mat_<uint8_t> & frame, const etld_object & object, const EtldClassifier & classifier, const EtldModel & model, const ETLDParams & params);
    int detect(const cv::Mat_<uint8_t> & frame, const etld_object & object, etld_detector_candidate * candidates, const EtldClassifier & classifier, const EtldModel & model);
private:
    int grid_step;
    float R_thrld;
    int D_thrld;
    int max_candidates_num;
    int w_scaled[DETECT_SCALES];
    int h_scaled[DETECT_SCALES];
    float detect_scales[DETECT_SCALES];
    float min_scale;
    float max_scale;
};
}
}

#endif // ETLDDETECTOR_H
