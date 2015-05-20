#include "shape.h"
#include <cmath>
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
Shape::Shape()
{
    m_is_valid = false;
}

Shape::Shape(vector<Point> &draw_contour)
{
    //Zwiększenie dokładności otoczki wypukłej
    vector<Point> tmp;
    double s = 10.0;
    double dx, dy;
    if(draw_contour.size() < 40)
    {
        for(int i = 0; i < draw_contour.size() - 1; i++)
        {
            dx = (double)(draw_contour[i+1].x - draw_contour[i].x) / s;
            dy = (double)(draw_contour[i+1].y - draw_contour[i].y) / s;
            for(int j = 0; j < s; j++)
                tmp.push_back(Point(draw_contour[i].x + dx * j , draw_contour[i].y + dy * j));
        }
        dx = (double)(draw_contour[0].x - draw_contour[draw_contour.size()-1].x) / s;
        dy = (double)(draw_contour[0].y - draw_contour[draw_contour.size()-1].y) / s;
        for(int j = 0; j < s; j++)
            tmp.push_back(Point(draw_contour[draw_contour.size()-1].x + dx * j , draw_contour[draw_contour.size()-1].y + dy * j));

        draw_contour = tmp;
    }

    vector<double> signature = shapeSignature(draw_contour);

    //Rysowanie sygnatury
    vector<Point> tmp_contours;
    for(int i = 0; i < signature.size(); i++)
    {
        tmp_contours.push_back(Point(i, signature[i] * 100));
    }

//    Mat t = Mat::zeros(300, 300, CV_8SC3);
//    drawSingleContour(t, tmp_contours, 1);
//    imshow("Sygnatura", t);
}

bool Shape::isValid()
{
    return m_is_valid;
}

vector<double> Shape::shapeSignature(vector<Point> &contour)
{
    int size = 100;
    vector<double> signature(size);
    for(double& d : signature) d = 0;

    Point center = FrameCalculation::contourCenter(contour);
    double a;
    double x, y;
    double d;
    int i;

    for(Point& p : contour)
    {
        x = p.x - center.x;
        y = p.y - center.y;
        if(fabs(x) < 0.00001) a = M_PI_2;
        else a = atan(fabs(y/x));
        if(x > 0 && y < 0) a += M_PI_2;
        else if(x > 0 && y > 0) a += M_PI;
        else if(x < 0 && y > 0) a += M_PI_2 * 3.0;

        d = sqrt(x*x + y*y);
        i = a / 2.0 / M_PI * size;
        cout << i << endl;
        if(signature[i] == 0) signature[i] = d;
        else signature[i] = (signature[i] + d) / 2;
    }
    //znajdowanie indeksu maksimum
    int index_max = 0;
    double max = signature[0];
    for(int i = 1; i < size; i++)
    {
        if(signature[i] > max)
        {
            max = signature[i];
            index_max = i;
        }
    }
    if(max < 0.00001) max = 1; //unikamy dzielenie przez 0
    //przesunięcie cykliczne tak aby maksimum było na początku oraz normalizacja do [0; 1]
    vector<double> tmp;
    for(int i = 0; i < index_max; i++)
        tmp.push_back(signature[i]);

    for(int i = 0; i + index_max < size; i++)
        signature[i] = signature[i+index_max] / max;

    for(int i = size - index_max; i < size; i++)
        signature[i] = tmp[i - size + index_max] / max;

    //usuwanie zer przez aproksymację liniową
    int index1 = 0, index2 = 0;
    double delta = 0;
    for(int i = 0; i < size; i++)
    {
        if(signature[i] == 0)
        {
            index2 = i;
        }
        else
        {
            if(index1 != index2) //index1 - ostatni punkt niezerowy, index2 ostatni punkt zerowy
            {
                delta = (signature[index2 + 1] - signature[index1]) / (index2 - index1 + 1);
                for(int j = index1 + 1; j <= index2; j++)
                    signature[j] = signature[j-1] + delta;
            }
            index1 = index2 = i;
        }
    }

    return signature;
}

double Shape::pearsonCoefficient(vector<double> &s1, vector<double> &s2)
{
    if(s1.size() != s2.size()) return 0.0;
    double size = s1.size();
    double sum_XY = 0.0f, sum_X = 0.0f, sum_Y = 0.0f, sum_XS = 0.0f, sum_YS = 0.0f;
    for (int i = 0; i < size; ++i) {
            sum_XY += s1[i] * s2[i];
            sum_X += s1[i];
            sum_Y += s2[i];
            sum_XS += pow(s1[i], 2.0f);
            sum_YS += pow(s2[i], 2.0f);
    }

    double res = (size * sum_XY - sum_X * sum_Y) /
            sqrtf((size * sum_XS - pow(sum_X, 2.0f)) * (size * sum_YS - pow(sum_Y, 2.0f)));

    return fabs(res);
}
