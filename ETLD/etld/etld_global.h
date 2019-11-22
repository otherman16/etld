#ifndef ETLD_GLOBAL_H
#define ETLD_GLOBAL_H

#include <opencv2/opencv.hpp>

#include <stdint.h>

#define DETECT_SCALES       (3)
#define LEARN_SCALES        (3)
#define LEARN_ANGLES        (3)

#define ETLD_W (320)
#define ETLD_H (240)
#define ETLD_N (76800)
namespace cv
{
namespace etld
{

typedef struct
_etld_object {
    bool valid;
    bool tracker_reinit;
    bool update;
    float C;
    float R;
    cv::Rect2i window;
} etld_object;
typedef struct
_etld_tracker_candidate {
    bool success;
    float C;
    float R;
    cv::Rect2i window;
} etld_tracker_candidate;
typedef struct
_etld_detector_candidate {
    float C;
    float R;
    cv::Rect2i window;
} etld_detector_candidate;
typedef struct
_etld_cluster {
    float C;
    float R;
    int n;
    cv::Rect2i window;
    int * idx;
} etld_cluster;

}
}

#endif // ETLD_GLOBAL_H
