#include "etldframe.h"

#define MIN_W  (40)
#define MIN_H  (30)

#define GRID_X_STEPS (16)
#define GRID_Y_STEPS (16)

#define GRID_X_STEP  (ETLD_W / GRID_X_STEPS)
#define GRID_Y_STEP  (ETLD_H / GRID_Y_STEPS)

EtldFrame::EtldFrame()
{
    frame = nullptr;
    prev_frame = nullptr;
    subframe = new cv::Mat_<uint8_t>(ETLD_H, ETLD_W);
    prev_subframe = new cv::Mat_<uint8_t>(ETLD_H, ETLD_W);
    smoothed_subframe = new cv::Mat_<uint8_t>(ETLD_H, ETLD_W);
    prev_smoothed_subframe = new cv::Mat_<uint8_t>(ETLD_H, ETLD_W);
    _find = false;
}
EtldFrame::~EtldFrame()
{
    if(frame) delete frame;
    if(prev_frame) delete prev_frame;
    if(subframe) delete subframe;
    if(prev_subframe) delete prev_subframe;
    if(smoothed_subframe) delete smoothed_subframe;
    if(prev_smoothed_subframe) delete prev_smoothed_subframe;
}
void EtldFrame::init(cv::Rect_<int> & new_object, const cv::Mat_<uint8_t> & f, cv::Rect_<int> & roi)
{
    if(frame) delete frame;
    if(prev_frame) delete prev_frame;
    frame = new cv::Mat_<uint8_t>(f.size());
    prev_frame = new cv::Mat_<uint8_t>(f.size());

    this->roi = cv::Rect_<int>(0, 0, f.cols, f.rows);

    choose_next_subframe(new_object);

    convert_rect_to_local(new_object);

    roi = this->roi;

    f.copyTo(*prev_frame);
    f.copyTo(*frame);

    calculate_subframe();
    calculate_smoothed_subframe();
}
void EtldFrame::next(cv::Rect_<int> & object, cv::Rect_<int> & tracker, const cv::Mat_<uint8_t> & f, cv::Rect_<int> & roi)
{
    if( (frame == nullptr) || (prev_frame == nullptr) || (frame->cols != f.cols) || (frame->rows != f.rows) ) return;

    convert_rect_to_global(object);
    convert_rect_to_global(tracker);

    choose_next_subframe(object);

    convert_rect_to_local(object);
    convert_rect_to_local(tracker);

    roi = this->roi;

    uint8_t * tmp = prev_frame->data;
    prev_frame->data = frame->data;
    frame->data = tmp;
    f.copyTo(*frame);

    calculate_subframe();
    calculate_smoothed_subframe();
}
void EtldFrame::find(cv::Rect_<int> & object, cv::Rect_<int> & tracker, const cv::Mat_<uint8_t> & f, cv::Rect_<int> & roi)
{
    if( (frame == nullptr) || (prev_frame == nullptr) || (frame->cols != f.cols) || (frame->rows != f.rows) ) return;

    convert_rect_to_global(object);
    convert_rect_to_global(tracker);

    choose_find_subframe(object);

    convert_rect_to_local(object);
    convert_rect_to_local(tracker);

    roi = this->roi;

    uint8_t * tmp = prev_frame->data;
    prev_frame->data = frame->data;
    frame->data = tmp;
    f.copyTo(*frame);

    calculate_subframe();
    calculate_smoothed_subframe();
}
cv::Mat_<uint8_t> & EtldFrame::get_prev_subframe()
{
    return *prev_smoothed_subframe;
}
cv::Mat_<uint8_t> & EtldFrame::get_subframe()
{
    return *smoothed_subframe;
}
cv::Mat_<uint8_t> & EtldFrame::get_smoothed_subframe()
{
    return *smoothed_subframe;
}
void EtldFrame::convert_rect_to_global(cv::Rect_<int> & r)
{
    float x_ratio = float(roi.width)  / float(subframe->cols);
    float y_ratio = float(roi.height) / float(subframe->rows);
    r.x      = int(roundf(float(r.x       * x_ratio) + roi.x));
    r.y      = int(roundf(float(r.y       * y_ratio) + roi.y));
    r.width  = int(roundf(float(r.width   * x_ratio)        ));
    r.height = int(roundf(float(r.height  * y_ratio)        ));
}
void EtldFrame::convert_rect_to_local(cv::Rect_<int> & r)
{
    float x_ratio = float(subframe->cols) / float(roi.width);
    float y_ratio = float(subframe->rows) / float(roi.height);
    r.x      = int(roundf(float((r.x - roi.x) * x_ratio)));
    r.y      = int(roundf(float((r.y - roi.y) * y_ratio)));
    r.width  = int(roundf(float((r.width    ) * x_ratio)));
    r.height = int(roundf(float((r.height   ) * y_ratio)));
}
void EtldFrame::choose_next_subframe(const cv::Rect_<int> & obj)
{
    _find = false;
    int _W = obj.width * 3;
    int _H = obj.height * 3;
    int FW = frame->cols;
    int FH = frame->rows;
    int W = roi.width;
    int H = roi.height;

    if(_W * 0.75f < _H)
    {
        _W = int(roundf(_H / 0.75f));
    }
    else
    {
        _H = int(roundf(_W * 0.75f));
    }

    if( (W < (_W - GRID_X_STEP)) || (W > (_W + GRID_X_STEP)) || (H < (_H - GRID_Y_STEP)) || (H > _H + GRID_Y_STEP) )
    {
        W = (_W > FW) ? FW : (_W < MIN_W) ? MIN_W :  ((_W / GRID_X_STEP) * GRID_X_STEP);
        H = (_H > FH) ? FH : (_H < MIN_H) ? MIN_H : ((_H / GRID_Y_STEP) * GRID_Y_STEP);
    }
    int xc = (obj.x + obj.x + obj.width) / 2;
    int _X0 = xc - W / 2;
    int _X1 = xc + W / 2;
    if(_X0 < 0) _X0 = 0;
    else if(_X1 >= FW) _X0 = FW - W;
    int yc = (obj.y + obj.y + obj.height) / 2;
    int _Y0 = yc - H / 2;
    int _Y1 = yc + H / 2;
    if(_Y0 < 0) _Y0 = 0;
    else if(_Y1 >= FH) _Y0 = FH - H;
    roi.x = _X0;
    roi.y = _Y0;
    roi.width = W;
    roi.height = H;
}
void EtldFrame::choose_find_subframe(const cv::Rect_<int> &)
{
    static uint32_t frames_object_invalid = 0;
    if(_find)
    {
//        X0 += (W * 3 / 4);
//        if(X0 == (FW - W / 4))
//        {
//            X0 = 0;
//            Y0 += (H * 3 / 4);
//            if(Y0 == (FH - H / 4))
//            {
//                Y0 = 0;
//            }
//            else if(Y0 > (FH - H))
//            {
//                Y0 = FH - H;
//            }
//        }
//        else if(X0 > (FW - W))
//        {
//            X0 = FW - W;
//        }
    }
    else
    {
        if(++frames_object_invalid > 10)
        {
            _find = true;
            frames_object_invalid = 0;
        }
    }
}
void EtldFrame::calculate_smoothed_subframe()
{
    cv::blur(*subframe, *smoothed_subframe, cv::Size(7, 7));
    cv::blur(*prev_subframe, *prev_smoothed_subframe, cv::Size(7, 7));
}
void EtldFrame::calculate_subframe()
{
    cv::resize((*frame)(roi), *subframe, subframe->size(), 0.0, 0.0, cv::INTER_NEAREST);
    cv::resize((*prev_frame)(roi), *prev_subframe, prev_subframe->size(), 0.0, 0.0, cv::INTER_NEAREST);
}
