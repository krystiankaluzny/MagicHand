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
    Shape();
    Shape(vector<Point>& draw_contour);

    bool isValid(); //jeżeli zidentyfikowane figurę

private:
    vector<double> shapeSignature(vector<Point> &contour);
    double pearsonCoefficient(vector<double>& s1, vector<double>& s2);

    bool m_is_valid;
};

#endif // SHAPE_H
