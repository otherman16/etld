#ifndef ETLDCLASSIFIER_H
#define ETLDCLASSIFIER_H

#include "etld/etld_global.h"
#include "etld/etldparams.h"
namespace cv
{
namespace etld
{
class EtldImage;
class EtldSimpleImage;
class EtldClassifier
{
public:
    EtldClassifier();
    EtldClassifier(const EtldClassifier &);
    ~EtldClassifier();

    EtldClassifier * copy();

    EtldClassifier & operator=(const EtldClassifier & c);

    void init(const cv::Mat_<uint8_t> & frame, const etld_object & object, const ETLDParams & params);

    void add_pos_ex(EtldImage & ex, const uint32_t & scale_idx = 0);
    void add_neg_ex(EtldImage & ex, const uint32_t & scale_idx = 0);
    void update();

    float R(EtldImage & p) const;
    float detect_R(EtldImage & p, const uint32_t & scale_idx = 0) const;
    float detect_R(EtldSimpleImage & p, const uint32_t & scale_idx = 0) const;
    float learn_R(EtldImage & p, const uint32_t & scale_idx = 0) const;
    void construct_learn_fern(const int & w, const int & h, const float * scales) const;
    void construct_detect_fern(const int & w, const int & h, const float * scales, const int & W) const;
private:
    inline int z(EtldImage & p, std::pair<cv::Point2i, cv::Point2i> * fern) const;
    inline int z(EtldImage & p, std::pair<int, int> * fern) const;
    inline int z(EtldSimpleImage & p, std::pair<cv::Point2i, cv::Point2i> * fern) const;
    inline int z(EtldSimpleImage & p, std::pair<int, int> * fern) const;
    void construct_origin_fern(const cv::Mat_<uint8_t> & frame, const etld_object & object);
    void allocate();
    void deallocate();
private:
    int fern_n;
    int fern_k;
    int fern_d;
    int fern_nd;
    int fern_nk;

    int * Np;
    int * Np_max;
    int * Nn;
    float * P;
    bool initialization;

    std::pair<cv::Point_<float>, cv::Point_<float>> * origin_fern;
    std::pair<cv::Point2i, cv::Point2i> * detect_fern;
    std::pair<int, int> * simple_detect_fern;
    std::pair<cv::Point2i, cv::Point2i> * learn_fern;
};
}
}

#endif // ETLDCLASSIFIER_H
