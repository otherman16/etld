#ifndef MY_FAST_H
#define MY_FAST_H

namespace cv
{
namespace etld
{
typedef struct { int x, y; } fast_xy;
typedef unsigned char fast_byte;

void my_fast_detect(const fast_byte* im, int xsize, int ysize, int stride, int b, fast_xy * corners, int max_corners, int* ret_num_corners);
}
}

#endif // MY_FAST_H
