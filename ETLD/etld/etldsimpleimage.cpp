#include "etldsimpleimage.h"

namespace cv
{
namespace etld
{
void EtldSimpleImage::copyTo(EtldSimpleImage & img)
{
    if((img.w() != _w) || (img.h() != _h)) return;
    memcpy(img.data, f, size_t(_n));
}
void EtldSimpleImage::bilin_scale(EtldSimpleImage & img)
{
    const int w = img.w();
    const int h = img.h();
    const float x_ratio = float(_w) / float(w);
    const float y_ratio = float(_h) / float(h);
    for(int y = 0; y < h; ++y)
    {
        float y_scaled = y * y_ratio;
        int y_scaled_int = int(y_scaled);
        float y_diff = y_scaled - y_scaled_int;
        for(int x = 0; x < w; ++x)
        {
            float x_scaled = x * x_ratio;
            int x_scaled_int = int(x_scaled);
            float x_diff = x_scaled - x_scaled_int;

            float A = float((*this)(x_scaled_int,     y_scaled_int));
            float B = float((*this)(x_scaled_int + 1, y_scaled_int));
            float C = float((*this)(x_scaled_int,     y_scaled_int + 1));
            float D = float((*this)(x_scaled_int + 1, y_scaled_int + 1));

            // Y = A(1-w)(1-h) + B(w)(1-h) + C(h)(1-w) + Dwh
            float Y = (
                ( ( ( ( A * (1 - x_diff) ) ) * (1 - y_diff) ) ) +
                ( ( ( ( B * (x_diff    ) ) ) * (1 - y_diff) ) ) +
                ( ( ( ( C * (1 - x_diff) ) ) * (y_diff    ) ) ) +
                ( ( ( ( D * (x_diff    ) ) ) * (y_diff    ) ) )
            );

            if(Y > 255) Y = 255;
            else if(Y < 0) Y = 0;

            img(x, y) = uint8_t(roundf(Y));
        }
    }
}
void EtldSimpleImage::bilin_rotate(EtldSimpleImage & img, const float & a)
{
    const int w = img.w();
    const int h = img.h();
    const float cx = w / 2.0f;
    const float cy = h / 2.0f;
    const float sina = sinf(a);
    const float cosa = cosf(a);
    for(int y = 0; y < h; ++y)
    {
        for(int x = 0; x < w; ++x)
        {
            float y_rotated = (cx - x) * sina + (y - cy) * cosa + cy;
            int y_rotated_int = int(y_rotated);
            float y_diff = y_rotated - y_rotated_int;
            float x_rotated = (x - cx) * cosa + (y - cy) * sina + cx;
            int x_rotated_int = int(x_rotated);
            float x_diff = x_rotated - x_rotated_int;

            float A = float((*this)(x_rotated_int,      y_rotated_int));
            float B = float((*this)(x_rotated_int + 1,  y_rotated_int));
            float C = float((*this)(x_rotated_int,      y_rotated_int + 1));
            float D = float((*this)(x_rotated_int + 1,  y_rotated_int + 1));

            // Y = A(1-w)(1-h) + B(w)(1-h) + C(h)(1-w) + Dwh
            float Y = (
                ( ( ( ( A * (1 - x_diff) ) ) * (1 - y_diff) ) ) +
                ( ( ( ( B * (x_diff    ) ) ) * (1 - y_diff) ) ) +
                ( ( ( ( C * (1 - x_diff) ) ) * (y_diff    ) ) ) +
                ( ( ( ( D * (x_diff    ) ) ) * (y_diff    ) ) )
            );

            if(Y > 255) Y = 255;
            else if(Y < 0) Y = 0;

            img(x, y) = uint8_t(roundf(Y));
        }
    }
}
void EtldSimpleImage::normalise()
{
    int m = M();
    for(int y = 0; y < _h; ++y)
    {
        for(int x = 0; x < _w; ++x)
        {
            int32_t tmp = int32_t((*this)(x, y)) - m;
            if(tmp < -128) tmp = -128;
            else if(tmp > 127) tmp = 127;
            (*this)(x, y) = uint8_t(tmp);
        }
    }
}
void EtldSimpleImage::set(const int & val)
{
    for(int y = 0; y < _h; ++y)
    {
        for(int x = 0; x < _w; ++x)
        {
            (*this)(x, y) = uint8_t(val);
        }
    }
}
float EtldSimpleImage::O(EtldSimpleImage & img)
{
    cv::Rect2i overlap_roi = this->roi() & img.roi();
    float s1 = float(this->roi().area());
    float s2 = float(img.roi().area());
    float s = float(overlap_roi.area());
    if(s == 0.0f) return 0.0f;
    float o = (s1 < s2) ? (s1 / s) : (s2 / s);
    return o;
}
int EtldSimpleImage::D()
{
    int m = 0, d = 0;
    for(int y = 0; y < _h; ++y)
    {
        for(int x = 0; x < _w; ++x)
        {
            int tmp = (*this)(x, y);
            m += tmp;
            d += tmp * tmp;
        }
    }
    m /= _n;
    d = d / _n - m * m;
    return d;
}
int EtldSimpleImage::M()
{
    int m = 0;
    for(int y = 0; y < _h; ++y)
    {
        for(int x = 0; x < _w; ++x)
        {
            m += (*this)(x, y);
        }
    }
    m /= _n;
    return m;
}
}
}
