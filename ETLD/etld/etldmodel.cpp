#include "etldmodel.h"
#include "etld/etldimage.h"
#include "etld/etldsimpleimage.h"

#include <math.h>

namespace cv
{
EtldModel::EtldModel()
{
    ex_w = 15;
    ex_h = 15;
    ex_wh = ex_w * ex_h;
    ex_n = 200;
    ex_const_n = 50;
    pos_ex_idx = 0;
    neg_ex_idx = 0;
    pos_ex_n = 0;
    neg_ex_n = 0;

    pos_ex = nullptr;
    neg_ex = nullptr;
}
EtldModel::EtldModel(const EtldModel & m)
{
    ex_w       = m.ex_w;
    ex_h       = m.ex_h;
    ex_wh      = ex_w * ex_h;
    ex_n       = m.ex_n;
    ex_const_n = m.ex_const_n;
    pos_ex_idx = m.pos_ex_idx;
    neg_ex_idx = m.neg_ex_idx;
    pos_ex_n   = m.pos_ex_n;
    neg_ex_n   = m.neg_ex_n;

    allocate();

    EtldImage ** _pos_ex = m.pos_ex;
    EtldImage ** _neg_ex = m.neg_ex;
    for(int i = 0; i < ex_n; ++i)
    {
        _pos_ex[i]->copyTo(*pos_ex[i]);
        _neg_ex[i]->copyTo(*neg_ex[i]);
    }
}
void EtldModel::allocate()
{
    pos_ex = new EtldImage * [ex_n];
    neg_ex = new EtldImage * [ex_n];
    for(int i = 0; i < ex_n; ++i)
    {
        pos_ex[i] = new EtldImage(ex_w, ex_h);
        pos_ex[i]->set(0);
        neg_ex[i] = new EtldImage(ex_w, ex_h);
        neg_ex[i]->set(0);
    }
}
EtldModel * EtldModel::copy()
{
    EtldModel * copy = new EtldModel(*this);
    return copy;
}
EtldModel & EtldModel::operator=(const EtldModel & m)
{
    deallocate();

    ex_w       = m.ex_w;
    ex_h       = m.ex_h;
    ex_wh      = ex_w * ex_h;
    ex_n       = m.ex_n;
    ex_const_n = m.ex_const_n;
    pos_ex_idx = m.pos_ex_idx;
    neg_ex_idx = m.neg_ex_idx;
    pos_ex_n   = m.pos_ex_n;
    neg_ex_n   = m.neg_ex_n;

    allocate();

    EtldImage ** _pos_ex = m.pos_ex;
    EtldImage ** _neg_ex = m.neg_ex;
    for(int i = 0; i < ex_n; ++i)
    {
        _pos_ex[i]->copyTo(*pos_ex[i]);
        _neg_ex[i]->copyTo(*neg_ex[i]);
    }

    return *this;
}
EtldImage ** EtldModel::get_pos_exs(int & n)
{
    n = pos_ex_n;
    return pos_ex;
}
EtldImage ** EtldModel::get_neg_exs(int & n)
{
    n = neg_ex_n;
    return neg_ex;
}
void EtldModel::init(const cv::Mat_<uint8_t> &, const etld_object & , const etld_settings & settings)
{
    deallocate();

    ex_w = settings.model_settings.ex_w;
    ex_h = settings.model_settings.ex_h;
    ex_wh = ex_w * ex_h;
    ex_n = settings.model_settings.ex_n;
    ex_const_n = settings.model_settings.ex_const_n;
    pos_ex_idx = 0;
    neg_ex_idx = 0;
    pos_ex_n = 0;
    neg_ex_n = 0;

    allocate();
}
void EtldModel::add_pos_ex(EtldImage & ex)
{
    ex.bilin_scale(*pos_ex[pos_ex_idx]);
    pos_ex[pos_ex_idx]->normalise();
    ++pos_ex_idx;
    if(pos_ex_n < ex_n) pos_ex_n = pos_ex_idx;
    if(pos_ex_idx == ex_n) pos_ex_idx = ex_const_n;
}
void EtldModel::add_neg_ex(EtldImage & ex)
{
    ex.bilin_scale(*neg_ex[neg_ex_idx]);
    neg_ex[neg_ex_idx]->normalise();
    ++neg_ex_idx;
    if(neg_ex_n < ex_n) neg_ex_n = neg_ex_idx;
    if(neg_ex_idx == ex_n) neg_ex_idx = ex_const_n;
}
float EtldModel::C(EtldImage & img) const
{
    EtldImage scaled_img(ex_w, ex_h);
    img.bilin_scale(scaled_img);
    scaled_img.normalise();
    // N
    float max_S_neg = 0;
    for(int i = 0; i < neg_ex_n; ++i)
    {
        float S_neg = S(scaled_img, *neg_ex[i]);
        if(S_neg > max_S_neg) max_S_neg = S_neg;
    }
    float N = 1 - max_S_neg;
    // P
    float max_S_pos = 0;
    for(int i = 0; i < pos_ex_n; ++i)
    {
        float S_pos = S(scaled_img, *pos_ex[i]);
        if(S_pos > max_S_pos) max_S_pos = S_pos;
    }
    float P = 1 - max_S_pos;
    // C
    if((N == 0.0f) || ((N + P) == 0.0f)) return 0.0f;
    float C = N / (N + P);
    return C;
}
float EtldModel::S(EtldImage & img1, EtldImage & img2) const
{
    int8_t * p1 = reinterpret_cast<int8_t *>(img1.data);
    int8_t * p2 = reinterpret_cast<int8_t *>(img2.data);
    size_t n = img1.total();
    float s1 = 0;
    float s2 = 0;
    float s3 = 0;
    for(size_t i = 0; i < n; ++i)
    {
        float p1_i = float(p1[i]);
        float p2_i = float(p2[i]);
        s1 += p1_i * p1_i;
        s2 += p2_i * p2_i;
        s3 += p1_i * p2_i;
    }
    if((s1 == 0.0f) || (s2 == 0.0f) || (s3 == 0.0f)) return 0;
    float NCC = s3 / sqrtf(s1 * s2);
    return NCC;
}
void EtldModel::deallocate()
{
    if(pos_ex)
    {
        for(int i = 0; i < ex_n; ++i)
        {
            delete pos_ex[i];
        }
        delete [] pos_ex;
        pos_ex = nullptr;
    }
    if(neg_ex)
    {
        for(int i = 0; i < ex_n; ++i)
        {
            delete neg_ex[i];
        }
        delete [] neg_ex;
        neg_ex = nullptr;
    }
}
EtldModel::~EtldModel()
{
    deallocate();
}
}
