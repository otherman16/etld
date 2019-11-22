#ifndef ETLDFRAME_H
#define ETLDFRAME_H

#include "etld/etld_global.h"
namespace cv
{
namespace etld
{
class EtldFrame
{
public:
    EtldFrame();
    ~EtldFrame();

    void init(cv::Rect2i & new_object, const cv::Mat & f, cv::Rect2i & roi);
    void next(cv::Rect2i & object, cv::Rect2i & tracker, const cv::Mat & f, cv::Rect2i & roi);
    void find(cv::Rect2i & object, cv::Rect2i & tracker, const cv::Mat & f, cv::Rect2i & roi);

    cv::Mat & get_prev_subframe();
    cv::Mat & get_subframe();
    cv::Mat & get_smoothed_subframe();
private:
    void convert_rect_to_global(cv::Rect2i & r);
    void convert_rect_to_local(cv::Rect2i & r);
    void choose_next_subframe(const cv::Rect2i & obj);
    void choose_find_subframe(const cv::Rect2i & obj);
    void calculate_subframe();
    void calculate_smoothed_subframe();
private:
    cv::Mat * frame;
    cv::Mat * prev_frame;
    cv::Mat * subframe;
    cv::Mat * prev_subframe;
    cv::Mat * smoothed_subframe;
    cv::Mat * prev_smoothed_subframe;
    cv::Rect2i roi;
    bool _find;
};
}
}

#endif // ETLDFRAME_H
