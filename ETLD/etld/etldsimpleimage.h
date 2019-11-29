#ifndef ETLDSIMPLEIMAGE_H
#define ETLDSIMPLEIMAGE_H

#include "etld/etld_global.h"
namespace cv
{
namespace etld
{
class EtldSimpleImage
{
private:
    const bool _is_fragment;
    uint8_t * f;
    const int _W;
    const int _H;
    const int _N;
    int _x;
    int _y;
    int _w;
    int _h;
    int _n;
    int _n0;
public:
    EtldSimpleImage(const int & w,
                    const int & h):
        _is_fragment(false),
        f(new uint8_t[w * h]),
        _W(w),
        _H(h),
        _N(w * h),
        _x(0),
        _y(0),
        _w(w),
        _h(h),
        _n(w * h),
        _n0(0),
        data(&f[0]){}
    EtldSimpleImage(uint8_t * f,
                    const int & W,
                    const int & H,
                    const int & x,
                    const int & y,
                    const int & w,
                    const int & h):
        _is_fragment(true),
        f(f),
        _W(W),
        _H(H),
        _N(W * H),
        _x(x),
        _y(y),
        _w(w),
        _h(h),
        _n(w * h),
        _n0(x + y * W),
        data(&f[x + y * W]){}
    EtldSimpleImage(const cv::Mat_<uint8_t> & frame,
                    const int & x,
                    const int & y,
                    const int & w,
                    const int & h):
        _is_fragment(true),
        f(frame.data),
        _W(frame.cols),
        _H(frame.rows),
        _N(frame.cols * frame.rows),
        _x(x),
        _y(y),
        _w(w),
        _h(h),
        _n(w * h),
        _n0(x + y * frame.cols),
        data(&f[x + y * frame.cols]){}
    EtldSimpleImage(const cv::Mat_<uint8_t> & frame,
                    const cv::Rect2i & roi):
        _is_fragment(true),
        f(frame.data),
        _W(frame.cols),
        _H(frame.rows),
        _N(frame.cols * frame.rows),
        _x(roi.x),
        _y(roi.y),
        _w(roi.width),
        _h(roi.height),
        _n(roi.width * roi.height),
        _n0(roi.x + roi.y * frame.cols),
        data(&f[roi.x + roi.y * frame.cols]){}
    ~EtldSimpleImage(){if(!_is_fragment) delete [] f;}

    bool is_fragment() const {return _is_fragment;}
    int W() const {return _W;}
    int H() const {return _H;}
    int N() const {return _N;}
    int x() const {return _x;}
    int y() const {return _y;}
    int w() const {return _w;}
    int h() const {return _h;}
    int n() const {return _n;}
    size_t total() const {return size_t(_n);}
    int n0() const {return _n0;}
    cv::Rect2i roi(){return cv::Rect2i(_x, _y, _w, _h);}

    uint8_t & operator[](const int & i){return f[_n0 + i];}
    uint8_t & operator()(const int & i){return f[_n0 + (i % _W) + (i / _W) * _W];}
    uint8_t & operator()(const int & x, const int & y){return f[_n0 + x + y * _W];}

    void copyTo(EtldSimpleImage & img);

    void set_size(const int & w, const int & h){_w = w;_h = h;_n = w * h;}
    void set_offset(const int & n0){_n0 = n0;_x = _n0 % _W;_y = _n0 / _W;}
    void set_offset(const int & x, const int & y){_x = x;_y = y;_n0 = x + y * _W;}

    void bilin_scale(EtldSimpleImage & img);
    void bilin_rotate(EtldSimpleImage & img, const float & a);
    void normalise();
    void set(const int & val);

    float O(EtldSimpleImage & img);
    int D();
    int M();
public:
    uint8_t * data;
};
}
}

#endif // ETLDSIMPLEIMAGE_H
