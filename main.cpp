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
    if(!cam.isOpened())
    {
        cerr << "Niemozna otworzyc kamerki.";
        return 1;
    }

    namedWindow("MagicHand", CV_WINDOW_AUTOSIZE);
    FrameCalculation fc("MagicHand");
//    FrameCalculation fc("MagicHand", true, 20);

    while(waitKey(3) != ' ')
    {
        if(!cam.read(current_frame)) break;
        flip(current_frame, current_frame, 1); //odwrucenie wokół pionu
        fc.calculate(current_frame);
    }

    return 0;
}

