#include <iostream>

using namespace std;

#include <opencv2/opencv.hpp>
#include <atomic>
#include <mutex>
#include <sstream>

#include "etld/etld.h"

static cv::Point2i pt0 = {0, 0}, pt1 = {0, 0};
static bool init = false, deinit = false;
static mutex mouse_callback_mutex;

static void mouse_callback(int event, int x, int y, int /*flags*/, void* /*userdata*/)
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
    cv::etld::etld_object obj;
    cv::Ptr<cv::etld::ETLD> etld = cv::etld::ETLD::create();
    cv::FileStorage fs_read("etld.xml", cv::FileStorage::READ);
    if(fs_read.isOpened())
    {
        etld->read(fs_read["etld_settings"]);
    }
    else
    {
        cv::FileStorage fs_write("etld.xml", cv::FileStorage::WRITE);
        etld->write(fs_write);
        fs_write.release();
    }
    fs_read.release();

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
                etld->deinit();
            }
            else if(init)
            {
                init = false;
                cv::Rect2d target(cv::min(pt0.x, pt1.x), cv::min(pt0.y, pt1.y), cv::abs(pt0.x - pt1.x), cv::abs(pt0.y - pt1.y));
                etld->init(gray, target);
            }
            else
            {
                cv::Rect2d target;
                bool valid = etld->update(gray, target);
                if(valid)
                {
                    cv::rectangle(frame, target, cv::Scalar(0, 255, 0), 2);
                    cv::etld::etld_tracker_candidate tc;
                    etld->get_tracker_candidates(&tc);
                    cv::rectangle(frame, tc.window, cv::Scalar(0, 255, 255), tc.success ? 2 : 1);
                }

                int etld_time = etld->etld_time();
                int frame_time = etld->frame_time();
                int init_time = etld->init_time();
                int detector_time = etld->detector_time();
                int tracker_time = etld->tracker_time();
                int integrator_time = etld->integrator_time();
                int update_time = etld->update_time();

                {
                    std::ostringstream os;
                    os << "etld_time " << etld_time;
                    cv::putText(frame, os.str(), cv::Point2i(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, 255);
                }
                {
                    std::ostringstream os;
                    os << "frame_time " << frame_time;
                    cv::putText(frame, os.str(), cv::Point2i(10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, 255);
                }
                {
                    std::ostringstream os;
                    os << "init_time " << init_time;
                    cv::putText(frame, os.str(), cv::Point2i(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.5, 255);
                }
                {
                    std::ostringstream os;
                    os << "detector_time " << detector_time;
                    cv::putText(frame, os.str(), cv::Point2i(10, 80), cv::FONT_HERSHEY_SIMPLEX, 0.5, 255);
                }
                {
                    std::ostringstream os;
                    os << "tracker_time " << tracker_time;
                    cv::putText(frame, os.str(), cv::Point2i(10, 100), cv::FONT_HERSHEY_SIMPLEX, 0.5, 255);
                }
                {
                    std::ostringstream os;
                    os << "integrator_time " << integrator_time;
                    cv::putText(frame, os.str(), cv::Point2i(10, 120), cv::FONT_HERSHEY_SIMPLEX, 0.5, 255);
                }
                {
                    std::ostringstream os;
                    os << "update_time " << update_time;
                    cv::putText(frame, os.str(), cv::Point2i(10, 140), cv::FONT_HERSHEY_SIMPLEX, 0.5, 255);
                }
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
