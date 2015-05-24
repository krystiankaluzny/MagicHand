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

    static double malinowskaCoefficient(vector<Point>& contours);
    static Point contourCenter(vector<Point> &contour);

    bool isValid(); //jeżeli zidentyfikowane figurę
    void draw(Mat& out);

    vector<Point> shape_contour;
    Point center;
    ShapeType type;

private:
    vector<double> shapeSignature(vector<Point> &contour, double *maximum = nullptr, int *index_of_max = nullptr);
    void increaseContourPrecision(vector<Point> &contour);
    double pearsonCoefficient(vector<double>& s1, vector<double>& s2);

    int m_size_signature;
};

#endif // SHAPE_H
