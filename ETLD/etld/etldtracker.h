#ifndef ETLDTRACKER_H
#define ETLDTRACKER_H

#include "etld/etld_global.h"

class EtldClassifier;
class EtldModel;

using namespace etld;
class EtldTracker
{
public:
    EtldTracker();
    ~EtldTracker();

    void init(const cv::Mat_<uint8_t> & frame, const etld_object & object, const EtldClassifier & classifier, const EtldModel & model, const etld_settings & settings);

    void track(const cv::Mat_<uint8_t> & prev_frame, const cv::Mat_<uint8_t> & frame, const cv::Mat_<uint8_t> & smoothed_frame, const etld_object & object, etld_tracker_candidate & candidate, const EtldClassifier & classifier, const EtldModel & model);

private:
    void count_pyramid(const cv::Mat_<uint8_t> & prev_frame, const cv::Mat_<uint8_t> & frame);
    void allocate(const cv::Size & size);
    void deallocate();
    bool track(float & x0, float & y0, float & w, float & h);

    void calcOpticalFlow(cv::Mat_<uint8_t> ** prevPyr, cv::Mat_<uint8_t> ** currPyr, std::vector<cv::Point2f> & pointsPrev, std::vector<cv::Point2f> & pointsCurr, std::vector<uchar> & status);

private:
    int pyramid_levels;
    cv::Mat_<uint8_t> ** pyramid_0;
    cv::Mat_<uint8_t> ** pyramid_1;
    cv::Mat_<uint8_t> ** cur_pyr;
    cv::Mat_<uint8_t> ** prev_pyr;

    int win_sz;
    int win_rad;
    int npts_width;
    int npts_height;
    int npts_n;
    int nmax_iter;
    float min_delta;
    int variance_thrld_div;
    int variance_thrld;
    int subgrid_sz;
    int subgrid_rad;
    float max_fb_err_coeff;
    float min_part_for_scale;
    bool use_fast;
};

#endif // ETLDTRACKER_H
