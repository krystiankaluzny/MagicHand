#ifndef CONTOUR_H
#define CONTOUR_H

#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

class Contour
{
public:
    Contour();

    static double malinowskaCoefficient(vector<Point> &contours);
    static Point contourCenterOfGravity(vector<Point> &contour);
    static Point contourCenter(vector<Point> &contour);
    static void increaseContourPrecision(vector<Point> &contour, int size);
    static double pearsonCoefficient(vector<double> &s1, vector<double> &s2);
    static void drawPoly(Mat& out, vector<Point>& poly, Point center, Scalar fill_color);
    static void sortPoints(vector<Point>& points);
};

#endif // CONTOUR_H
