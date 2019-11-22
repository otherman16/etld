#include "etldtracker.h"

#include "etld/etldimage.h"
#include "etld/etldsimpleimage.h"
#include "etld/etldclassifier.h"
#include "etld/etldmodel.h"
#include "etld/my_fast.h"

#include <math.h>
#include <cstring>

#include <vector>

namespace cv
{
namespace etld
{
template<typename T>
static T median(const std::vector<T> & _v)
{
    std::vector<T> v(_v);
    std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
    return v[v.size() / 2];
}
static float sqrEuclidDistance(const float & x0, const float & y0, const float & x1, const float & y1)
{
    return ( (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) );
}
static float bilinearInterpolation(const cv::Mat_<uint8_t> & img, const float & x, const float & y)
{
    if( (x < 0) || (x > (img.cols - 2)) || (y < 0) || (y > (img.rows - 2)) ) return 0;

    int32_t intX0 = int32_t(x);
    int32_t intX1 = intX0 + 1;
    int32_t intY0 = int32_t(y);
    int32_t intY1 = intY0 + 1;
    uint8_t * data = img.data;
    int32_t W = img.cols;

    float p1 = float(data[intY0 * W + intX0]) + (x - intX0) * (float(data[intY0 * W + intX1] - data[intY0 * W + intX0]));
    float p2 = float(data[intY1 * W + intX0]) + (x - intX0) * (float(data[intY1 * W + intX1] - data[intY1 * W + intX0]));
    return ( p1 + (y - intY0) * (p2 - p1) );
}
static int D(const cv::Mat_<uint8_t> & img, const float & x, const float & y, const int & winSz = 7)
{
    const int rad = winSz / 2;

    if( ((x - rad) < 0) || ((x + rad) > (img.cols - 1)) || ((y - rad) < 0) || ((y + rad) > (img.rows - 1)) ) return 0;

    cv::Scalar obj_mean, obj_stddev;
    cv::meanStdDev(cv::Mat_<uint8_t>(img, cv::Rect_<int>(int(x) - rad, int(y) - rad, winSz, winSz)), obj_mean, obj_stddev);
    int D = int(round(obj_stddev.val[0]));
    return D;
}

EtldTracker::EtldTracker()
{
    pyramid_levels = 0;
    pyramid_0 = nullptr;
    pyramid_1 = nullptr;

    win_sz              = 5;// winSz*winSz - размер окна суммирования для одной точки
    win_rad             = win_sz / 2;
    npts_width          = 10;// число точек в сетке по ширине
    npts_height         = 10;// число точек в сетке по высоте
    npts_n              = npts_width * npts_height;
    nmax_iter           = 20;// максимальное число итераций поиска оптического потока
    min_delta           = 0.03f;// минимальное значения смещения, после которого опт. поток считается посчитаным
    variance_thrld_div  = 20;//делитель порога по дисперсии для отсеивания точек
    variance_thrld      = 0;
    subgrid_sz          = 7;
    subgrid_rad         = subgrid_sz / 2;
    max_fb_err_coeff    = 0.05f;
    min_part_for_scale  = 0.1f;
    use_fast            = false;
}
EtldTracker::~EtldTracker()
{
    deallocate();
}
void EtldTracker::init(const cv::Mat_<uint8_t> & frame, const etld_object & , const EtldClassifier & , const EtldModel & , const ETLDParams & params)
{
    deallocate();

    pyramid_levels      = params.tracker_settings.pyramid_levels;
    win_sz              = params.tracker_settings.win_sz;
    win_rad             = win_sz / 2;
    npts_width          = params.tracker_settings.npts_width;
    npts_height         = params.tracker_settings.npts_height;
    npts_n              = npts_width * npts_height;
    nmax_iter           = params.tracker_settings.nmax_iter;
    min_delta           = params.tracker_settings.min_delta;
    variance_thrld_div  = params.tracker_settings.variance_thrld_div;
    variance_thrld      = 0;
    subgrid_sz          = params.tracker_settings.subgrid_sz;
    subgrid_rad         = subgrid_sz / 2;
    max_fb_err_coeff    = params.tracker_settings.max_fb_err_coeff;
    min_part_for_scale  = params.tracker_settings.min_part_for_scale;
    use_fast            = params.tracker_settings.use_fast;

    allocate(frame.size());

    count_pyramid(frame, frame);
}
void EtldTracker::count_pyramid(const cv::Mat_<uint8_t> & prev_frame, const cv::Mat_<uint8_t> & frame)
{
    prev_frame.copyTo(*prev_pyr[0]);
    frame.copyTo(*cur_pyr[0]);
    for(int level = 1; level < pyramid_levels; ++level)
    {
        cv::resize(*prev_pyr[level - 1], *prev_pyr[level], prev_pyr[level]->size(), 0.0, 0.0, cv::INTER_NEAREST);
        cv::resize(*cur_pyr[level - 1], *cur_pyr[level], cur_pyr[level]->size(), 0.0, 0.0, cv::INTER_NEAREST);
    }
}
void EtldTracker::allocate(const cv::Size & size)
{
    pyramid_0 = new cv::Mat_<uint8_t> * [pyramid_levels];
    pyramid_1 = new cv::Mat_<uint8_t> * [pyramid_levels];
    pyramid_0[0] = new cv::Mat_<uint8_t>(size);
    pyramid_1[0] = new cv::Mat_<uint8_t>(size);
    for(int i = 1; i < pyramid_levels; ++i)
    {
        pyramid_0[i] = new cv::Mat_<uint8_t>(pyramid_0[i - 1]->size() / 2);
        pyramid_1[i] = new cv::Mat_<uint8_t>(pyramid_1[i - 1]->size() / 2);
    }
    cur_pyr = pyramid_0;
    prev_pyr = pyramid_1;
}
void EtldTracker::deallocate()
{
    if(pyramid_0)
    {
        for(int i = 0; i < pyramid_levels; ++i)
        {
            delete pyramid_0[i];
        }
        delete [] pyramid_0;
        pyramid_0 = nullptr;
    }
    if(pyramid_1)
    {
        for(int i = 0; i < pyramid_levels; ++i)
        {
            delete pyramid_1[i];
        }
        delete [] pyramid_1;
        pyramid_1 = nullptr;
    }
}
void EtldTracker::track(const cv::Mat_<uint8_t> & prev_frame, const cv::Mat_<uint8_t> & frame, const cv::Mat_<uint8_t> & smoothed_frame, const etld_object & object, etld_tracker_candidate & candidate, const EtldClassifier & , const EtldModel & model)
{
    if( !object.valid )
    {
        candidate.success = false;
        return;
    }
    count_pyramid(prev_frame, frame);

    float x0_float = float(candidate.window.x);
    float y0_float = float(candidate.window.y);
    float w_float  = float(candidate.window.width);
    float h_float  = float(candidate.window.height);

    if(object.tracker_reinit)
    {
        x0_float = float(object.window.x);
        y0_float = float(object.window.y);
        w_float  = float(object.window.width);
        h_float  = float(object.window.height);
    }

    if( candidate.success || object.tracker_reinit )
    {
        EtldImage obj(prev_frame, object.window);
        variance_thrld = obj.D() / variance_thrld_div;
        candidate.success = track(x0_float, y0_float, w_float, h_float);
        if( (x0_float < 0) || (y0_float < 0) || ( (x0_float + w_float) >= frame.cols ) || ( (y0_float + h_float) >= frame.rows ) )
        {
            candidate.success = false;
        }
        if(candidate.success)
        {
            candidate.window.x      = int(roundf(x0_float));
            candidate.window.y      = int(roundf(y0_float));
            candidate.window.width  = int(roundf(w_float));
            candidate.window.height = int(roundf(h_float));
            EtldImage obj(smoothed_frame, candidate.window);
            candidate.C = model.C(obj);
            candidate.C = (int32_t(candidate.C * 100.0f)) / 100.0f;
            candidate.R = 0;
        }
    }
}
void EtldTracker::calcOpticalFlow(cv::Mat_<uint8_t> ** prev_pyr, cv::Mat_<uint8_t> ** cur_pyr, std::vector<cv::Point2f> & pointsPrev, std::vector<cv::Point2f> & pointsCurr, std::vector<uchar> & status)
{
    float * subGrid = new float[subgrid_sz * subgrid_sz];
    float * dIdxS = new float[win_sz * win_sz];
    float * dIdyS = new float[win_sz * win_sz];

    // цикл по уровням пирамиды
    for(int lvl = (pyramid_levels - 1); lvl >= 0; --lvl)
    {
        int scale = 1 << lvl;

        // цикл по точкам
        for(size_t n = 0; n < status.size(); ++n)
        {
            if(!status[n]) continue;

            cv::Point2f pointPrev(pointsPrev[n].x / scale, pointsPrev[n].y / scale);

            // Count SubGrid
            for(int j = 0; j < subgrid_sz; ++j)
            {
                for(int i = 0; i < subgrid_sz; ++i)
                {
                    subGrid[j * subgrid_sz + i] =  bilinearInterpolation(*prev_pyr[lvl], pointPrev.x - subgrid_rad + i, pointPrev.y - subgrid_rad + j);
                }
            }

            float sum_dIdxSQR  = 0;
            float sum_dIdySQR  = 0;
            float sum_dIdxdIdy = 0;
            float sum_IdIdx    = 0;
            float sum_IdIdy    = 0;

            // цикл подсчета сумм производных по окну winSz*winSz
            for(int j = 1; j < (subgrid_sz - 1); ++j)
            {
                for(int i = 1; i < (subgrid_sz - 1); ++i)
                {
                    float dIdx, dIdy;
                    if(
                        (
                            ( (pointPrev.x + (i + 1 - (subgrid_rad))) <= (prev_pyr[lvl]->cols - 1) ) ||
                            ( (pointPrev.x + (i - 1 - (subgrid_rad))) >= 0 )
                         ) && (
                            ( (pointPrev.y + (j + 1 - (subgrid_rad))) <= (prev_pyr[lvl]->rows - 1) ) ||
                            ( (pointPrev.y + (j - 1 - (subgrid_rad))) >= 0 )
                         )
                      )
                    {
                        dIdx = (subGrid[ j      * subgrid_sz + i + 1] - subGrid[ j      * subgrid_sz + i - 1]) / 2;
                        dIdy = (subGrid[(j + 1) * subgrid_sz + i    ] - subGrid[(j - 1) * subgrid_sz + i    ]) / 2;
                    }
                    else
                    {
                        dIdx = 0;
                        dIdy = 0;
                    }

                    float I = subGrid[j * subgrid_sz + i];
                    sum_dIdxSQR  += dIdx * dIdx;
                    sum_dIdySQR  += dIdy * dIdy;
                    sum_dIdxdIdy += dIdx * dIdy;
                    sum_IdIdx    +=    I * dIdx;
                    sum_IdIdy    +=    I * dIdy;
                    dIdxS[(j - 1) * win_sz + (i - 1)] = dIdx;
                    dIdyS[(j - 1) * win_sz + (i - 1)] = dIdy;
                }
            }

            float det = sum_dIdxSQR * sum_dIdySQR - sum_dIdxdIdy * sum_dIdxdIdy;

            if( (-0.0001f < det) && (det < 0.0001f) ) continue; // det == 0

            cv::Point2f pointCurr(pointsCurr[n].x / scale, pointsCurr[n].y / scale);

            // цикл итераций поиска оптического потока
            for(int k = 0; k < nmax_iter; k++)
            {
                float sum_JdJdx = 0;
                float sum_JdJdy = 0;

                // цикл подсчета столбца свободных членов
                for(int j = 0; j < win_sz; ++j)
                {
                    for(int i = 0; i < win_sz; ++i)
                    {
                        float J = bilinearInterpolation(*cur_pyr[lvl], pointCurr.x - win_rad + i, pointCurr.y - win_rad + j);
                        sum_JdJdx += J * dIdxS[j * win_sz + i];
                        sum_JdJdy += J * dIdyS[j * win_sz + i];
                    }
                }

                float sum_dIdtdIdx = sum_JdJdx - sum_IdIdx;
                float sum_dIdtdIdy = sum_JdJdy - sum_IdIdy;

                float deltaX = (sum_dIdtdIdy * sum_dIdxdIdy - sum_dIdySQR * sum_dIdtdIdx) / det;
                float deltaY = (sum_dIdtdIdx * sum_dIdxdIdy - sum_dIdxSQR * sum_dIdtdIdy) / det;

                if( ( (deltaX >= 0 && deltaX < float(min_delta)) || (deltaX < 0 && deltaX > float(-min_delta))    ) &&
                    ( (deltaY >= 0 && deltaY < float(min_delta)) || (deltaY < 0 && deltaY > float(-min_delta))    )    ) // выход из цикла, если поток сошелся
                    break;

                pointCurr.x += deltaX;
                pointCurr.y += deltaY;

                if( pointCurr.x < 0 || pointCurr.x >= (cur_pyr[lvl]->cols  - 1) || pointCurr.y < 0 || pointCurr.y >= (cur_pyr[lvl]->rows - 1)   )
                {
                    status[n] = false;
                    break;
                }
            }
            pointsCurr[n].x = pointCurr.x * scale;
            pointsCurr[n].y = pointCurr.y * scale;
        }
        // конец цикла по точкам
    }

    delete [] subGrid;
    delete [] dIdxS;
    delete [] dIdyS;
}
bool EtldTracker::track (float & x0, float & y0, float & w, float & h)
{
    std::vector<cv::Point2f> points[3];
    std::vector<uchar> status;
    std::vector<float> deltaXVector;
    std::vector<float> deltaYVector;
    std::vector<float> scaleVector;
    std::vector<float> fbError;
    std::vector<size_t> idx;

    if(use_fast)
    {
        int retNumPoints;
        fast_xy * sp = new fast_xy[npts_n];
        fast_byte * im = reinterpret_cast<fast_byte *>(prev_pyr[0]->data);
        my_fast_detect(im + uint32_t(x0) + uint32_t(y0 * cur_pyr[0]->cols), int32_t(w), int32_t(h), int32_t(prev_pyr[0]->cols), int32_t(variance_thrld), sp, int32_t(npts_n), &retNumPoints);
        if(retNumPoints > 50)
        {
            for(int i = 0; i < retNumPoints; ++i)
            {
                float x = x0 + sp[i].x;
                float y = y0 + sp[i].y;
                points[0].push_back(cv::Point2f(x, y));
                points[1].push_back(cv::Point2f(x, y));
                points[2].push_back(cv::Point2f(x, y));
                status.push_back(true);
            }
        }
        delete [] sp;
    }
    if( !use_fast || (status.size() == 0) )
    {
        float stepW = w  / (npts_width  - 1);
        float stepH = h / (npts_height - 1);

        for(int j = 0; j < npts_height; ++j)
        {
            float y = y0 + j * stepH;
            for(int i = 0; i < npts_width; ++i)
            {
                float x = x0 + i * stepW;
                int var = D(*cur_pyr[0], x, y);
                if( var > variance_thrld )
                {
                    points[0].push_back(cv::Point2f(x, y));
                    points[1].push_back(cv::Point2f(x, y));
                    points[2].push_back(cv::Point2f(x, y));
                    status.push_back(true);
                }
            }
        }
    }

    if(status.size() == 0)
    {
        return false;
    }

    //************************************************************************************
    calcOpticalFlow(prev_pyr, cur_pyr, points[0], points[1], status);
    calcOpticalFlow(cur_pyr, prev_pyr, points[1], points[2], status);
    //************************************************************************************
//    std::vector<float> err;
//    cv::TermCriteria termcrit(cv::TermCriteria::COUNT|cv::TermCriteria::EPS, nmax_iter, min_delta);
//    cv::calcOpticalFlowPyrLK(*prev_pyr[0], *cur_pyr[0], points[0], points[1], status, err, cv::Size(npts_width, npts_height), pyramid_levels, termcrit, 0, 0.0001);
//    cv::calcOpticalFlowPyrLK(*cur_pyr[0], *prev_pyr[0], points[1], points[2], status, err, cv::Size(npts_width, npts_height), pyramid_levels, termcrit, 0, 0.0001);
    //************************************************************************************

    for(size_t i = 0; i < status.size(); ++i)
    {
        if(status[i])
        {
            float a = points[0][i].x - points[2][i].x;
            float b = points[0][i].y - points[2][i].y;
            fbError.push_back(sqrt(a * a + b * b));
        }
    }

    if(fbError.size() == 0)
    {
        return false;
    }

    size_t medianInd = fbError.size() / 2; // количество точек, прошедшее определение потока два раза, деленое на два
    float fbErrMedianThr = max_fb_err_coeff * sqrtf(w * h);
    float fbErrMedian = median(fbError);

    if( (medianInd > 0) && (fbErrMedian < fbErrMedianThr) )
    {
        for(size_t i = 0; i < status.size(); ++i)
        {
            if( status[i] && (fbError[i] <= fbErrMedian) )
            {
                deltaXVector.push_back(points[1][i].x - points[0][i].x);
                deltaYVector.push_back(points[1][i].y - points[0][i].y);
                idx.push_back(i);
            }
        }

        medianInd = idx.size() / 2;
        if(medianInd > min_part_for_scale * status.size())
        {
            float deltaX = median(deltaXVector);
            float deltaY = median(deltaYVector);

            for(size_t i = 0; i < idx.size(); i++)
            {
                for(size_t j = idx.size() - 1; j > i; j--)
                {
                    float sqrDistPrev = sqrEuclidDistance(points[0][idx[i]].x, points[0][idx[i]].y, points[0][idx[j]].x, points[0][idx[j]].y);
                    float sqrDistCurr = sqrEuclidDistance(points[1][idx[i]].x, points[1][idx[i]].y, points[1][idx[j]].x, points[1][idx[j]].y);
                    scaleVector.push_back(sqrtf(sqrDistCurr/ sqrDistPrev));
                }
            }
            float scale = median(scaleVector);

            float xc = x0 + w / 2;
            float yc = y0 + h / 2;
            xc += deltaX;
            yc += deltaY;
            h *= scale;
            w *= scale;
            x0 = xc - w / 2;
            y0 = yc - h / 2;
        }
        else if(medianInd > 2)
        {
            float deltaX = median(deltaXVector);
            float deltaY = median(deltaYVector);
            float xc = x0 + w / 2;
            float yc = y0 + h / 2;
            xc += deltaX;
            yc += deltaY;
            x0 = xc - w / 2;
            y0 = yc - h / 2;
        }
        else
        {
            return false;
        }
    }

    return true;
}
}
}
