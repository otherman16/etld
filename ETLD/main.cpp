#include <iostream>

using namespace std;

#include <opencv2/opencv.hpp>
#include <atomic>
#include <mutex>

#include "etld/etld.h"

static cv::Point2i pt0 = {0, 0}, pt1 = {0, 0};
static bool init = false, deinit = false;
static mutex mouse_callback_mutex;

static void mouse_callback(int event, int x, int y, int flags, void* userdata)
{
    switch(event)
    {
    case cv::EVENT_LBUTTONDOWN:
        mouse_callback_mutex.lock();
        init = false;
        deinit = false;
        pt0 = {x, y};
        mouse_callback_mutex.unlock();
        break;
    case cv::EVENT_LBUTTONUP:
        mouse_callback_mutex.lock();
        init = true;
        deinit = false;
        pt1 = {x, y};
        mouse_callback_mutex.unlock();
        break;
    case cv::EVENT_MBUTTONUP:
        mouse_callback_mutex.lock();
        init = false;
        deinit = true;
        mouse_callback_mutex.unlock();
        break;
    }
}

int main()
{
    int device_idx = 0;
    string device_name;
    cv::VideoCapture video_capture;
    if(device_name.empty())
    {
        video_capture.open(device_idx);
    }
    else
    {
        video_capture.open(device_name);
    }
    if(!video_capture.isOpened())
    {
        cout << "Device opening error " << device_name << " : " << device_idx << endl;
        cv::destroyAllWindows();
        return 1;
    }
    cv::namedWindow("Frame", cv::WINDOW_NORMAL);
    cv::setMouseCallback("Frame", mouse_callback, nullptr);
    cv::Mat frame, gray;
    cv::Rect_<int> etld_roi;
    cv::etld_object obj;
    cv::ETLD etld;

    while(true)
    {
        video_capture >> frame;
        if(!frame.empty())
        {
            cv::resize(frame, frame, cv::Size(1920, 1440), 0, 0, cv::INTER_CUBIC);
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

//            cv::Mat noise(gray.size(), gray.type());
//            cv::Mat m(1, 1, CV_32FC1);
//            m = cv::Scalar(2);
//            cv::Mat sigma(1, 1, CV_32FC1);
//            sigma = cv::Scalar(4);
//            cv::randn(noise, m, sigma);
//            cv::Mat ngray = gray + noise;
//            cv::imshow("ngray", ngray);

            mouse_callback_mutex.lock();
            if(deinit)
            {
                deinit = false;
                etld.deinit();
            }
            else if(init)
            {
                init = false;
                cv::Rect target(cv::min(pt0.x, pt1.x), cv::min(pt0.y, pt1.y), cv::abs(pt0.x - pt1.x), cv::abs(pt0.y - pt1.y));
                etld.init(target);
            }
            if(etld.isOn())
            {
                etld.new_frame(gray, etld_roi);
                etld.get_object(&obj);
                if(obj.valid)
                {
                    cv::rectangle(frame, obj.window, cv::Scalar(0, 255, 0), 2);
                }
                cout << etld.str_timings() << endl;
            }
            mouse_callback_mutex.unlock();

            cv::imshow("Frame", frame);
        }

        int key = cv::waitKey(1);
        if(key == 27)
        {
            break;
        }
    }

    return 0;
}
