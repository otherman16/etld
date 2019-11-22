#ifndef ETLDMODEL_H
#define ETLDMODEL_H

#include "etld/etld_global.h"
#include "etld/etldparams.h"

namespace cv
{
namespace etld
{
class EtldImage;
class EtldModel
{
public:
    EtldModel();
    EtldModel(const EtldModel &);
    ~EtldModel();

    EtldModel * copy();

    EtldModel & operator=(const EtldModel & m);

    void init(const cv::Mat_<uint8_t> & frame, const etld_object & object, const ETLDParams & params);
    void add_pos_ex(EtldImage &);
    void add_neg_ex(EtldImage &);

    float C(EtldImage &) const;

    EtldImage ** get_pos_exs(int & n);
    EtldImage ** get_neg_exs(int & n);

private:
    float S(EtldImage & img1, EtldImage & img2) const;
    void allocate();
    void deallocate();
private:
    int ex_w;
    int ex_h;
    int ex_wh;
    int ex_n;
    int ex_const_n;
    int pos_ex_idx;
    int neg_ex_idx;
    int pos_ex_n;
    int neg_ex_n;
    EtldImage ** pos_ex;
    EtldImage ** neg_ex;
};
}
}

#endif // ETLDMODEL_H
