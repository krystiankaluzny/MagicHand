#ifndef SHAPE_H
#define SHAPE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

#include "framecalculation.h"

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
    void draw(Mat& out);
    void moveTo(Point& point);

    vector<Point> shape_contour;
    Point center;
    ShapeType type;

private:
    vector<double> shapeSignature(vector<Point> &contour, double *maximum = nullptr, int *index_of_max = nullptr);
    void identifyShape(vector<Point> &draw_contour);

    int m_size_signature;
};

#endif // SHAPE_H
