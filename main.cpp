#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include "framecalculation.h"

using namespace std;
using namespace cv;

Mat current_frame;

int main()
{

    VideoCapture cam;
    cam.open(0);
//    namedWindow("MagicHand", CV_WINDOW_AUTOSIZE);
    FrameCalculation fc("MagicHand");

    if(!cam.isOpened()) return 1;

    while(waitKey(2) != ' ')
    {
        cam >> current_frame;
        flip(current_frame, current_frame, 1); //odwrucenie wokół pionu
        fc.calculate(current_frame);
    }

    return 0;
}

