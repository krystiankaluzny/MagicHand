#ifndef FRAMECALCULATION_H
#define FRAMECALCULATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fstream>

#include "shape.h"

using namespace std;
using namespace cv;

class Shape;

class FrameCalculation
{
public:
    FrameCalculation(string window_name);
    ~FrameCalculation();

    void calculate(Mat& frame);

    static Point contourCenter(vector<Point>& contour);

private:
    void calibration(Mat& frame);
    void selecPointers(Mat& frame);
    void identifyShape();

    double malinowskaCoefficient(double L, double S);
    string intToString(int num);
    Vec3i HSVToRGB(Vec3i &hsv);
    int countMaskPixels(Mat& mask);
    void drawSingleContour(Mat& out, vector<Point>& contour, int size, bool close = false);

    string window_name;

    Mat output;
    Mat hsv;
    Mat binary;
//    Mat out, buffor;
    Mat morphology_element;

    Vec3i average_color;
    Vec3i lower_color;
    Vec3i upper_color;
    Vec3i rgb_color;

    vector<Shape*> shapes;

    vector< vector<Point> > pointers_contours;
    vector< vector<Point> > contours; //kontury pomocnicze
    vector<Point> draw_points; //punkty narysowane przez urzytkownika
//    vector<Point> draw_contour; //

    bool calibrated;
    bool drawing;
    bool identified;
    int calibration_windows_size;
    int calibration_counter;
    int blink_counter; //zliczanie liczby kolejnych ramek na których nie ma żadnego
    int min_countur_size;
};

#endif // FRAMECALCULATION_H
