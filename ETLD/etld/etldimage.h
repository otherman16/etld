#ifndef ETLDIMAGE_H
#define ETLDIMAGE_H

#include "etld/etld_global.h"

using namespace etld;
class EtldImage : public cv::Mat_<uint8_t>
{
public:
    EtldImage(const int & w,
              const int & h);
    EtldImage(const cv::Mat_<uint8_t> & frame,
              const cv::Rect_<int> & _roi);
    EtldImage(const cv::Mat_<uint8_t> & frame,
              const int & x,
              const int & y,
              const int & w,
              const int & h);
    EtldImage(uint8_t * f,
              const int & W,
              const int & H,
              const int & x,
              const int & y,
              const int & w,
              const int & h);

    int w() const {return cols;}
    int h() const {return rows;}
    cv::Rect_<int> & roi(){return _roi;}

    uint8_t & operator[](const int & idx){return data[idx];}
    uint8_t & operator()(const int & idx){return data[idx];}
    uint8_t & operator()(const int & x, const int & y){return at<uint8_t>(y, x);}

    void bilin_scale(EtldImage & scaled_img);
    void bilin_rotate(EtldImage & rotated_img, const float & a);

    void normalise();
    void set(const int & val);

    float O(EtldImage & img);
    int M();
    int D();
private:
    cv::Rect_<int> _roi;
};

#endif // ETLDIMAGE_H
