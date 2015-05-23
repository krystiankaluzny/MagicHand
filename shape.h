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
        ST_SQUARE = 1,
        ST_CIRCLE = 2,
        ST_TRIANGLE = 3,
        ST_RIGTH_TRIANGLE = 4,

        ST_NONE
    };

    Shape();
    Shape(vector<Point>& draw_contour);

    bool isValid(); //jeżeli zidentyfikowane figurę
    vector<Point> shape_contour;

private:
    vector<double> shapeSignature(vector<Point> &contour, double *maximum = nullptr, int *index_of_max = nullptr);
    void increaseContourPrecision(vector<Point> &contour);
    double pearsonCoefficient(vector<double>& s1, vector<double>& s2);

    ShapeType m_type;
    int m_size_signature;
};

#endif // SHAPE_H
