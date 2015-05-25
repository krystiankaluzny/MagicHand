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
//    cam.open("data/video/prostokat.avi");
//    cam.open("data/video/trojkat.avi");
//    cam.open("data/video/okrag.avi");
//    cam.open("data/video/rozne.avi");
//    cam.open("data/video/zamykanie.avi");
//        cam.open("data/video/y.avi");
    if(!cam.isOpened())
    {
        cerr << "Niemozna otworzyc kamerki.";
        return 1;
    }

//    VideoWriter video("data/video/rozne2.avi",(int)cam.get(CV_CAP_PROP_FOURCC), 16, Size((int)cam.get(CV_CAP_PROP_FRAME_WIDTH),(int)cam.get(CV_CAP_PROP_FRAME_HEIGHT)) );
//    VideoWriter video("data/video/zamykanie.avi",(int)cam.get(CV_CAP_PROP_FOURCC), 16, Size((int)cam.get(CV_CAP_PROP_FRAME_WIDTH),(int)cam.get(CV_CAP_PROP_FRAME_HEIGHT)) );
//    if(!video.isOpened())
//    {
//        cerr << "Nie mozna zapisac do pliku.";
//        return 2;
//    }
    namedWindow("MagicHand", CV_WINDOW_AUTOSIZE);
    FrameCalculation fc("MagicHand");

    while(waitKey(2) != ' ')
    {
        if(!cam.read(current_frame)) break;
//        video << current_frame;
        flip(current_frame, current_frame, 1); //odwrucenie wokół pionu
        fc.calculate(current_frame);
    }

    return 0;
}

