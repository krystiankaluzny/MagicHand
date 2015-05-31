#include "contour.h"
#include <cmath>
#include <iostream>
#include <algorithm>

#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923

Contour::Contour()
{
}

double Contour::malinowskaCoefficient(vector<Point> &contours)
{
    double S = contourArea(contours), L = arcLength(contours, true);
    if(S < 0.00001) return 10;
    return L / (2 * sqrt(M_PI * S)) - 1;
}

Point Contour::contourCenterOfGravity(vector<Point> &contour)
{
    if(contour.size() == 0) return Point(-1, -1);
    double x = 0, y = 0;
    vector<Point>::iterator iter;
    for(iter = contour.begin(); iter != contour.end(); iter++)
    {
        x += (*iter).x;
        y += (*iter).y;
    }
    x /= contour.size();
    y /= contour.size();

    return Point(x , y);
}
Point Contour::contourCenter(vector<Point> &contour)
{
    if(contour.size() == 0) return Point(-1, -1);

    Moments mu = moments(contour, false );
    return Point(mu.m10/mu.m00, mu.m01/mu.m00);
}

void Contour::increaseContourPrecision(vector<Point> &contour, int size)
{
    vector<Point> tmp;
    double s;
    double dx, dy;
    if(contour.size() > 0 && contour.size() < size)
    {
        s = size / contour.size() * 1.2;
        for(int i = 0; i < contour.size() - 1; i++)
        {
            dx = (double)(contour[i+1].x - contour[i].x) / s;
            dy = (double)(contour[i+1].y - contour[i].y) / s;
            for(int j = 0; j < s; j++)
            {
                tmp.push_back(Point(contour[i].x + dx * j, contour[i].y + dy * j));
            }
        }
        dx = (double)(contour[0].x - contour[contour.size()-1].x) / s;
        dy = (double)(contour[0].y - contour[contour.size()-1].y) / s;
        for(int j = 0; j < s; j++)
            tmp.push_back(Point(contour[contour.size()-1].x + dx * j , contour[contour.size()-1].y + dy * j));

        contour = tmp;
    }
}

double Contour::pearsonCoefficient(vector<double> &s1, vector<double> &s2)
{
    if(s1.size() != s2.size()) return 0.0;
    if(s1.size() == 0) return 0.0;

    double size = s1.size();
    double sum_XY = 0.0f, sum_X = 0.0f, sum_Y = 0.0f, sum_XS = 0.0f, sum_YS = 0.0f;
    for (int i = 0; i < size; ++i)
    {
        sum_XY += s1[i] * s2[i];
        sum_X += s1[i];
        sum_Y += s2[i];
        sum_XS += pow(s1[i], 2.0f);
        sum_YS += pow(s2[i], 2.0f);
    }

    double res = (size * sum_XY - sum_X * sum_Y) /
            sqrt((size * sum_XS - pow(sum_X, 2.0f)) * (size * sum_YS - pow(sum_Y, 2.0f)));

    return fabs(res);
}

void Contour::drawPoly(Mat &out, vector<Point> &poly, Point center, Scalar fill_color)
{
    Scalar color2(fill_color[0]-40 > 0 ? fill_color[0]-40 : 1,
                  fill_color[1]-40 > 0 ? fill_color[1]-40 : 1,
                  fill_color[2]-40 > 0 ? fill_color[2]-40 : 1);

    fillPoly(out, vector< vector<Point> >(1,poly), fill_color);
    polylines(out, vector< vector<Point> >(1,poly), true, color2, 2);
    if(center != Point(-1, -1))
        circle(out, center, 10, color2, -1);
}

void Contour::sortPoints(vector<Point> &points)
{
    if(points.size() == 0) return;
    Point c =  Contour::contourCenterOfGravity(points);
    auto compare = [&c](Point a, Point b)->bool
    {
        double fi1, fi2;
        fi1 =  atan2(a.y - c.y, a.x - c.x);
        fi2 =  atan2(b.y - c.y, b.x - c.x);
        return fi1 > fi2;
    };
    sort(points.begin(), points.end(), compare);
}

Mat Contour::alphaBlend(const Mat &src1, const Mat &src2, float alpha)
{
    if(src1.size != src2.size || src1.type() != src2.type()) return Mat();
    Mat out(src1.rows, src1.cols, src1.type());
    Vec3b color1, color2;
    if(alpha > 1.0) alpha = 1.0;
    else if(alpha < 0.0) alpha = 0.0;
    float beta = 1.0 - alpha;

    for(int i = 0; i < src1.rows; i++)
    {
        for(int j = 0; j < src1.cols; j++)
        {
            color1 = src1.at<Vec3b>(i, j);
            color2 = src2.at<Vec3b>(i, j);
            if(color1 == Vec3b(0, 0, 0))  out.at<Vec3b>(i, j) = color2;
            else if(color2 == Vec3b(0, 0, 0)) out.at<Vec3b>(i, j) = color1;
            else out.at<Vec3b>(i, j) = Vec3b(beta * color1[0] + alpha * color2[0],
                                             beta * color1[1] + alpha * color2[1],
                                             beta * color1[2] + alpha * color2[2]);
        }
    }
    return out;
}

void Contour::alphaBlendMask(Mat &dst, Mat &intensity_mask, Scalar color, Point offset, float alpha)
{
    if(alpha > 1.0) alpha = 1.0;
    else if(alpha < 0.0) alpha = 0.0;
    float beta = 1.0 - alpha;
    Vec3b color1, color2;
    float intensity;
    for(int i = offset.y, k = 0; i < dst.rows && k < intensity_mask.rows; ++i, ++k)
    {
        for(int j = offset.x, l = 0; j < dst.cols && l < intensity_mask.cols; ++j, ++l)
        {
            intensity = (float)intensity_mask.at<uchar>(k, l) / 255.0;
            color1 = dst.at<Vec3b>(i, j);
            color2 = Vec3b(intensity * color[0], intensity * color[1], intensity * color[2]);
            if(color1 == Vec3b(0, 0, 0))  dst.at<Vec3b>(i, j) = color2;
            else if(color2 == Vec3b(0, 0, 0)) dst.at<Vec3b>(i, j) = color1;
            else dst.at<Vec3b>(i, j) = Vec3b(beta * color1[0] + alpha * color2[0],
                                             beta * color1[1] + alpha * color2[1],
                                             beta * color1[2] + alpha * color2[2]);
        }
    }
}

