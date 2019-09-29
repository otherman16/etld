#ifndef ETLDFRAME_H
#define ETLDFRAME_H

#include "etld/etld_global.h"
using namespace etld;
class EtldFrame
{
public:
    EtldFrame();
    ~EtldFrame();

    void init(cv::Rect_<int> & new_object, const cv::Mat_<uint8_t> & f, cv::Rect_<int> & roi);
    void next(cv::Rect_<int> & object, cv::Rect_<int> & tracker, const cv::Mat_<uint8_t> & f, cv::Rect_<int> & roi);
    void find(cv::Rect_<int> & object, cv::Rect_<int> & tracker, const cv::Mat_<uint8_t> & f, cv::Rect_<int> & roi);

    cv::Mat_<uint8_t> & get_prev_subframe();
    cv::Mat_<uint8_t> & get_subframe();
    cv::Mat_<uint8_t> & get_smoothed_subframe();
private:
    void convert_rect_to_global(cv::Rect_<int> & r);
    void convert_rect_to_local(cv::Rect_<int> & r);
    void choose_next_subframe(const cv::Rect_<int> & obj);
    void choose_find_subframe(const cv::Rect_<int> & obj);
    void calculate_subframe();
    void calculate_smoothed_subframe();
private:
    cv::Mat_<uint8_t> * frame;
    cv::Mat_<uint8_t> * prev_frame;
    cv::Mat_<uint8_t> * subframe;
    cv::Mat_<uint8_t> * prev_subframe;
    cv::Mat_<uint8_t> * smoothed_subframe;
    cv::Mat_<uint8_t> * prev_smoothed_subframe;
    cv::Rect_<int> roi;
    bool _find;
};

#endif // ETLDFRAME_H
