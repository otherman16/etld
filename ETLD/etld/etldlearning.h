#ifndef ETLDLEARNING_H
#define ETLDLEARNING_H

#include "etld/etld_global.h"

class EtldImage;
class EtldClassifier;
class EtldModel;

using namespace etld;
class EtldLearning
{
public:
    EtldLearning();
    ~EtldLearning();

    void init(const cv::Mat_<uint8_t> & frame, const etld_object & object, EtldClassifier & classifier, EtldModel & model, const etld_settings & settings);
    void update(const cv::Mat_<uint8_t> & frame, const etld_object & object, EtldClassifier & classifier, EtldModel & model);
private:
    int m_pos_num;
    int m_neg_num;
    int c_pos_num;
    int c_neg_num;
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
    int detector_grid_step;
    float learn_scales[LEARN_SCALES];
    float learn_angles[LEARN_ANGLES];
};

#endif // ETLDLEARNING_H
