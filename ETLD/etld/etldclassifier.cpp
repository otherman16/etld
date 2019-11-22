#include "etldclassifier.h"
#include "etld/etldimage.h"
#include "etld/etldsimpleimage.h"

#include <random>
#include <math.h>
#include <cstring>

#define INIT_NP_THR_DIV (3)

namespace cv
{
EtldClassifier::EtldClassifier()
{
    fern_n         = 10;
    fern_k         = 10;
    fern_d         = int(pow(2, fern_k));
    fern_nd        = fern_n * fern_d;
    fern_nk        = fern_n * fern_k;
    initialization = false;
    Np             = nullptr;
    Np_max         = nullptr;
    Nn             = nullptr;
    P              = nullptr;
    origin_fern    = nullptr;
    detect_fern    = nullptr;
    simple_detect_fern    = nullptr;
    learn_fern     = nullptr;
}
EtldClassifier::EtldClassifier(const EtldClassifier & c)
{
    fern_n         = c.fern_n;
    fern_k         = c.fern_k;
    fern_d         = c.fern_d;
    fern_nd        = c.fern_nd;
    fern_nk        = c.fern_nk;
    initialization = c.initialization;

    allocate();

    memcpy(Np,          c.Np,          sizeof(int) * size_t(fern_nd));
    memcpy(Np_max,      c.Np_max,      sizeof(int) * size_t(fern_n));
    memcpy(Nn,          c.Nn,          sizeof(int) * size_t(fern_nd));
    memcpy(P,           c.P,           sizeof(float) * size_t(fern_nd));
    memcpy(origin_fern, c.origin_fern, sizeof(std::pair<cv::Point_<float>, cv::Point_<float>>) * size_t(fern_nk));
    memcpy(detect_fern, c.detect_fern, sizeof(std::pair<cv::Point_<int>, cv::Point_<int>>) * size_t(fern_nk) * DETECT_SCALES);
    memcpy(simple_detect_fern, c.simple_detect_fern, sizeof(std::pair<int, int>) * size_t(fern_nk) * DETECT_SCALES);
    memcpy(learn_fern,  c.learn_fern,  sizeof(std::pair<cv::Point_<int>, cv::Point_<int>>) * size_t(fern_nk) * LEARN_SCALES);
}
void EtldClassifier::allocate()
{
    Np          = new int[fern_nd];
    Np_max      = new int[fern_n];
    Nn          = new int[fern_nd];
    P           = new float[fern_nd];
    origin_fern = new std::pair<cv::Point_<float>, cv::Point_<float>>[fern_nk];
    detect_fern = new std::pair<cv::Point_<int>, cv::Point_<int>>[fern_nk * DETECT_SCALES];
    simple_detect_fern = new std::pair<int, int>[fern_nk * DETECT_SCALES];
    learn_fern  = new std::pair<cv::Point_<int>, cv::Point_<int>>[fern_nk * LEARN_SCALES];

    memset(Np,     0, size_t(fern_nd) * sizeof(int));
    memset(Nn,     0, size_t(fern_nd) * sizeof(int));
    memset(P,      0, size_t(fern_nd) * sizeof(float));
    memset(Np_max, 0, size_t(fern_n ) * sizeof(int));
}
EtldClassifier * EtldClassifier::copy()
{
    EtldClassifier * copy = new EtldClassifier(*this);
    return copy;
}
EtldClassifier & EtldClassifier::operator=(const EtldClassifier & c)
{
    deallocate();

    fern_n         = c.fern_n;
    fern_k         = c.fern_k;
    fern_d         = c.fern_d;
    fern_nd        = c.fern_nd;
    fern_nk        = c.fern_nk;
    initialization = c.initialization;

    allocate();

    memcpy(Np,          c.Np,          sizeof(int) * size_t(fern_nd));
    memcpy(Np_max,      c.Np_max,      sizeof(int) * size_t(fern_n));
    memcpy(Nn,          c.Nn,          sizeof(int) * size_t(fern_nd));
    memcpy(P,           c.P,           sizeof(float) * size_t(fern_nd));
    memcpy(origin_fern, c.origin_fern, sizeof(std::pair<cv::Point_<float>, cv::Point_<float>>) * size_t(fern_nk));
    memcpy(detect_fern, c.detect_fern, sizeof(std::pair<cv::Point_<int>, cv::Point_<int>>) * size_t(fern_nk) * DETECT_SCALES);
    memcpy(simple_detect_fern, c.simple_detect_fern, sizeof(std::pair<int, int>) * size_t(fern_nk) * DETECT_SCALES);
    memcpy(learn_fern,  c.learn_fern,  sizeof(std::pair<cv::Point_<int>, cv::Point_<int>>) * size_t(fern_nk) * LEARN_SCALES);

    return *this;
}
void EtldClassifier::init(const cv::Mat_<uint8_t> & frame, const etld_object & object, const etld_settings & settings)
{
    deallocate();

    fern_n         = settings.classifier_settings.fern_n;
    fern_k         = settings.classifier_settings.fern_k;
    fern_d         = int(pow(2, fern_k));
    fern_nd        = fern_n * fern_d;
    fern_nk        = fern_n * fern_k;
    initialization = true;

    allocate();

    construct_origin_fern(frame, object);
}
void EtldClassifier::construct_detect_fern(const int & w, const int & h, const float * scales, const int & W) const
{
    std::pair<cv::Point_<int>, cv::Point_<int>> * detect_fern_i = detect_fern;
    std::pair<int, int> * simple_detect_fern_i = simple_detect_fern;
    for(int scale_idx = 0; scale_idx < DETECT_SCALES; ++scale_idx)
    {
        int scaled_w = int(roundf(w * scales[scale_idx]));
        int scaled_h = int(roundf(h * scales[scale_idx]));
        for(int nk = 0; nk < fern_nk; ++nk)
        {
            detect_fern_i[nk].first.x  = int(roundf(scaled_w * origin_fern[nk].first.x ));
            detect_fern_i[nk].first.y  = int(roundf(scaled_h * origin_fern[nk].first.y ));
            detect_fern_i[nk].second.x = int(roundf(scaled_w * origin_fern[nk].second.x));
            detect_fern_i[nk].second.y = int(roundf(scaled_h * origin_fern[nk].second.y));
            simple_detect_fern_i[nk].first  = detect_fern_i[nk].first.x  + detect_fern_i[nk].first.y  * W;
            simple_detect_fern_i[nk].second = detect_fern_i[nk].second.x + detect_fern_i[nk].second.y * W;
        }
        detect_fern_i += fern_nk;
        simple_detect_fern_i += fern_nk;
    }
}
void EtldClassifier::construct_learn_fern(const int & w, const int & h, const float * scales) const
{
    std::pair<cv::Point_<int>, cv::Point_<int>> * learn_fern_i = learn_fern;
    for(int scale_idx = 0; scale_idx < LEARN_SCALES; ++scale_idx)
    {
        int scaled_w = int(roundf(w * scales[scale_idx]));
        int scaled_h = int(roundf(h * scales[scale_idx]));
        for(int nk = 0; nk < fern_nk; ++nk)
        {
            learn_fern_i[nk].first.x  = int(roundf(scaled_w * origin_fern[nk].first.x ));
            learn_fern_i[nk].first.y  = int(roundf(scaled_h * origin_fern[nk].first.y ));
            learn_fern_i[nk].second.x = int(roundf(scaled_w * origin_fern[nk].second.x));
            learn_fern_i[nk].second.y = int(roundf(scaled_h * origin_fern[nk].second.y));
        }
        learn_fern_i += fern_nk;
    }
}
void EtldClassifier::add_pos_ex(EtldImage & ex, const uint32_t & scale_idx)
{
    std::pair<cv::Point_<int>, cv::Point_<int>> * learn_fern_n = learn_fern + size_t(fern_nk) * scale_idx;
    int * Np_i = Np;
    for(int n = 0; n < fern_n; ++n)
    {
        int _z = z(ex, learn_fern_n);
        ++Np_i[_z];
        if(initialization)
        {
            int tmp = Np_i[_z];
            if(tmp > Np_max[n]) Np_max[n] = tmp;
        }
        learn_fern_n += fern_k;
        Np_i += fern_d;
    }
}
void EtldClassifier::add_neg_ex(EtldImage & ex, const uint32_t & scale_idx)
{
    std::pair<cv::Point_<int>, cv::Point_<int>> * learn_fern_n = learn_fern + size_t(fern_nk) * scale_idx;
    int * Nn_i = Nn;
    for(int n = 0; n < fern_n; ++n)
    {
        int _z = z(ex, learn_fern_n);
        if(initialization)
        {
            if( Nn_i[_z] < (Np_max[n] / INIT_NP_THR_DIV) ) ++Nn_i[_z];
        }
        else
        {
            ++Nn_i[_z];
        }
        learn_fern_n += fern_k;
        Nn_i += fern_d;
    }
}
void EtldClassifier::update()
{
    for(int i = 0; i < fern_nd; ++i)
    {
        float tmp = ((Np[i] + Nn[i]) == 0) ? 0 : ( float(Np[i]) / float(Np[i] + Nn[i]) );
        P[i] = tmp;
    }
    initialization = false;
}
float EtldClassifier::R(EtldImage & p) const
{
    std::pair<cv::Point_<int>, cv::Point_<int>> * fern = new std::pair<cv::Point_<int>, cv::Point_<int>>[fern_nk];
    int w = p.w();
    int h = p.h();
    for(int nk = 0; nk < fern_nk; ++nk)
    {
        fern[nk].first.x  = int(roundf(w * origin_fern[nk].first.x ));
        fern[nk].first.y  = int(roundf(h * origin_fern[nk].first.y ));
        fern[nk].second.x = int(roundf(w * origin_fern[nk].second.x));
        fern[nk].second.y = int(roundf(h * origin_fern[nk].second.y));
    }
    std::pair<cv::Point_<int>, cv::Point_<int>> * fern_i = fern;
    float * P_i = P;
    float r = 0;
    for(int i = 0; i < fern_n; ++i)
    {
        int _z = z(p, fern_i);
        r += P_i[_z];
        fern_i += fern_k;
        P_i += fern_d;
    }
    r /= fern_n;
    delete [] fern;
    return r;
}
float EtldClassifier::detect_R(EtldImage & p, const uint32_t & scale_idx) const
{
    std::pair<cv::Point_<int>, cv::Point_<int>> * detect_fern_n = detect_fern + size_t(fern_nk) * scale_idx;
    float * P_i = P;
    float r = 0;
    for(int n = 0; n < fern_n; ++n)
    {
        int _z = z(p, detect_fern_n);
        r += P_i[_z];
        detect_fern_n += fern_k;
        P_i += fern_d;
    }
    r /= fern_n;
    return r;
}
float EtldClassifier::detect_R(EtldSimpleImage & p, const uint32_t & scale_idx) const
{
    std::pair<int, int> * detect_fern_n = simple_detect_fern + size_t(fern_nk) * scale_idx;
    float * P_i = P;
    float r = 0;
    for(int n = 0; n < fern_n; ++n)
    {
        int _z = z(p, detect_fern_n);
        r += P_i[_z];
        detect_fern_n += fern_k;
        P_i += fern_d;
    }
    r /= fern_n;
    return r;
}
float EtldClassifier::learn_R(EtldImage & p, const uint32_t & scale_idx) const
{
    std::pair<cv::Point_<int>, cv::Point_<int>> * learn_fern_n = learn_fern + size_t(fern_nk) * scale_idx;
    float * P_i = P;
    float r = 0;
    for(int n = 0; n < fern_n; ++n)
    {
        int _z = z(p, learn_fern_n);
        r += P_i[_z];
        learn_fern_n += fern_k;
        P_i += fern_d;
    }
    r /= fern_n;
    return r;
}
inline int EtldClassifier::z(EtldImage & p, std::pair<cv::Point_<int>, cv::Point_<int>> * fern) const
{
    int z = 0;
    for(int k = 0; k < fern_k; ++k)
    {
        z <<= 1;
        z |= ( p(fern[k].first.x, fern[k].first.y) > p(fern[k].second.x, fern[k].second.y) ) ? 1 : 0;
    }
    return z;
}
inline int EtldClassifier::z(EtldImage & p, std::pair<int, int> * fern) const
{
    int z = 0;
    for(int k = 0; k < fern_k; ++k)
    {
        z <<= 1;
        z |= ( p[fern[k].first] > p[fern[k].second] ) ? 1 : 0;
    }
    return z;
}
inline int EtldClassifier::z(EtldSimpleImage & p, std::pair<cv::Point_<int>, cv::Point_<int>> * fern) const
{
    int z = 0;
    for(int k = 0; k < fern_k; ++k)
    {
        z <<= 1;
        z |= ( p(fern[k].first.x, fern[k].first.y) > p(fern[k].second.x, fern[k].second.y) ) ? 1 : 0;
    }
    return z;
}
inline int EtldClassifier::z(EtldSimpleImage & p, std::pair<int, int> * fern) const
{
    int z = 0;
    for(int k = 0; k < fern_k; ++k)
    {
        z <<= 1;
        z |= ( p[fern[k].first] > p[fern[k].second] ) ? 1 : 0;
    }
    return z;
}
void EtldClassifier::construct_origin_fern(const cv::Mat_<uint8_t> & frame, const etld_object & object)
{
    const int w = object.window.width;
    const int h = object.window.height;
    EtldImage obj(frame, object.window);
    std::pair<cv::Point_<float>, cv::Point_<float>> * origin_fern_i = origin_fern;
    for(int n = 0; n < fern_n; ++n)
    {
        for(int k = 0, n = 0; k < fern_k / 4; ++k, ++n)
        {
            float x1 = float((rand() % 256) / 256.0);
            float y1 = float((rand() % 256) / 256.0);
            float x2 = x1;
            float y2 = float((rand() % 256) / 256.0);
            if((n < 1000) && abs(obj(int(roundf(x1 * w)), int(roundf(y1 * h))) - obj(int(roundf(x2 * w)), int(roundf(y2 * h)))) < 8)
            {
                --k;
                continue;
            }
            origin_fern_i[k].first.x = x1;
            origin_fern_i[k].first.y = y1;
            origin_fern_i[k].second.x = x2;
            origin_fern_i[k].second.y = y2;
        }
        for(int k = fern_k / 4, n = 0; k < fern_k / 2; ++k, ++n)
        {
            float x1 = float((rand() % 256) / 256.0);
            float y1 = float((rand() % 256) / 256.0);
            float x2 = x1;
            float y2 = float((rand() % 256) / 256.0);
            origin_fern_i[k].first.x = x1;
            origin_fern_i[k].first.y = y1;
            origin_fern_i[k].second.x = x2;
            origin_fern_i[k].second.y = y2;
        }
        for(int k = fern_k / 2, n = 0; k < fern_k * 3 / 4; ++k, ++n)
        {
            float x1 = float((rand() % 256) / 256.0);
            float y1 = float((rand() % 256) / 256.0);
            float x2 = float((rand() % 256) / 256.0);
            float y2 = y1;
            if((n < 1000) && abs(obj(int(roundf(x1 * w)), int32_t(int(y1 * h))) - obj(int(roundf(x2 * w)), int(roundf(y2 * h)))) < 8)
            {
                --k;
                continue;
            }
            origin_fern_i[k].first.x = x1;
            origin_fern_i[k].first.y = y1;
            origin_fern_i[k].second.x = x2;
            origin_fern_i[k].second.y = y2;
        }
        for(int k = fern_k * 3 / 4, n = 0; k < fern_k; ++k, ++n)
        {
            float x1 = float((rand() % 256) / 256.0);
            float y1 = float((rand() % 256) / 256.0);
            float x2 = float((rand() % 256) / 256.0);
            float y2 = y1;
            origin_fern_i[k].first.x = x1;
            origin_fern_i[k].first.y = y1;
            origin_fern_i[k].second.x = x2;
            origin_fern_i[k].second.y = y2;
        }
        origin_fern_i += fern_k;
    }
}
void EtldClassifier::deallocate()
{
    if(Np)          delete [] Np;
    if(Np_max)      delete [] Np_max;
    if(Nn)          delete [] Nn;
    if(P)           delete [] P;
    if(origin_fern) delete [] origin_fern;
    if(detect_fern) delete [] detect_fern;
    if(simple_detect_fern) delete [] simple_detect_fern;
    if(learn_fern)  delete [] learn_fern;
    Np              = nullptr;
    Np_max          = nullptr;
    Nn              = nullptr;
    P               = nullptr;
    origin_fern     = nullptr;
    detect_fern     = nullptr;
    simple_detect_fern = nullptr;
    learn_fern      = nullptr;
}
EtldClassifier::~EtldClassifier()
{
    deallocate();
}
}
