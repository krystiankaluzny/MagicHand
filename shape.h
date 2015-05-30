#ifndef SHAPE_H
#define SHAPE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

#include "contour.h"

using namespace std;
using namespace cv;

class Shape
{
public:
    enum ShapeType
    {
        ST_RECTANGLE = 0,
        ST_CIRCLE,
        ST_TRIANGLE,

        ST_NONE
    };

    Shape();
    Shape(vector<Point>& draw_contour);

    bool isValid(); //jeżeli zidentyfikowane figurę
    void drawShape(Mat& out, int dt);
    void moveShapeTo(Point& point);
    bool removeShape(vector<Point> &pointers, int dt);

    Point center;

private:
    vector<double> shapeSignature(vector<Point> &contour, double *maximum = nullptr, int *index_of_max = nullptr);
    void identifyAndApproximation(vector<Point> &draw_contour);
    void createMask(); //tworzenie maski intensywności kolorów figury

    vector<Point> shape_contour;
    int size_signature;
    int erasing_time;
    int max_erasing_time;
    ShapeType type;
    Point offset;
    Mat intensity_mask;
};

#endif // SHAPE_H
