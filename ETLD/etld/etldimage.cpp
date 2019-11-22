#include "etldimage.h"

namespace cv
{
EtldImage::EtldImage(const int & w,
                     const int & h) :
    cv::Mat_<uint8_t>(h, w),
    _roi(cv::Rect_<int>(0, 0, w, h))
{
}
EtldImage::EtldImage(const cv::Mat_<uint8_t> & frame,
                     const cv::Rect_<int> & roi):
    cv::Mat_<uint8_t>(frame, roi),
    _roi(roi)
{
}
EtldImage::EtldImage(const cv::Mat_<uint8_t> & frame,
                     const int & x,
                     const int & y,
                     const int & w,
                     const int & h):
    cv::Mat_<uint8_t>(frame, cv::Rect_<int>(x, y, w, h)),
    _roi(cv::Rect_<int>(x, y, w, h))
{
}
EtldImage::EtldImage(uint8_t * f,
                     const int & W,
                     const int & H,
                     const int & x,
                     const int & y,
                     const int & w,
                     const int & h):
    cv::Mat_<uint8_t>(cv::Mat_<uint8_t>(int(H), int(W), f, cv::Mat::AUTO_STEP), cv::Rect_<int>(x, y, w, h)),
    _roi(cv::Rect_<int>(x, y, w, h))
{
}
void EtldImage::bilin_scale(EtldImage & scaled_img)
{
    cv::resize(*this, scaled_img, scaled_img.size(), 0.0, 0.0, cv::INTER_NEAREST);
}
void EtldImage::bilin_rotate(EtldImage & rotated_img, const float & a)
{
    cv::Mat M = cv::getRotationMatrix2D(cv::Point2f(cols / 2, rows / 2), double(a), 1.0);
    cv::warpAffine(*this, rotated_img, M, rotated_img.size());
}
void EtldImage::normalise()
{
    int m = M();
    for(size_t i = 0; i < total(); ++i)
    {
        int32_t tmp = int32_t(data[i]) - m;
        if(tmp < -128) tmp = -128;
        else if(tmp > 127) tmp = 127;
        data[i] = uint8_t(tmp);
    }
}
void EtldImage::set(const int & val)
{
    memset(data, val, total());
}
float EtldImage::O(EtldImage & img)
{
    cv::Rect_<int> overlap_roi = _roi & img.roi();
    float s1 = float(_roi.area());
    float s2 = float(img.roi().area());
    float s = float(overlap_roi.area());
    if(s == 0.0f) return 0.0f;
    float o = (s1 < s2) ? (s1 / s) : (s2 / s);
    return o;
}
int EtldImage::M()
{
    cv::Scalar obj_mean, obj_stddev;
    cv::meanStdDev(*this, obj_mean, obj_stddev);
    int M = int(round(obj_mean.val[0]));
    return M;
}
int EtldImage::D()
{
    cv::Scalar obj_mean, obj_stddev;
    cv::meanStdDev(*this, obj_mean, obj_stddev);
    int D = int(round(obj_stddev.val[0]));
    return D;
}
}
