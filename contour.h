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

    static double malinowskaCoefficient(vector<Point>& contours);                                       //wyliczanie wspóczynnika Malinowskiej
    static Point contourCenterOfGravity(vector<Point>& contour);                                        //środek ciężkości figury
    static Point contourCenter(vector<Point>& contour);                                                 //środek geometryczny figury
    static void increaseContourPrecision(vector<Point>& contour, int size);                             //zwiększanie liczby punktów na bokach konturu tak aby ich suma był przynajmniej równa size
    static double pearsonCoefficient(vector<double> &s1, vector<double> &s2);                           //wyznacznei współczynnika korelacji Pearsona
    static void drawPoly(Mat& out, vector<Point>& poly, Point center, Scalar fill_color);               //narysowanie wypełnionej figury, jej przegu oraz środka
    static void sortPoints(vector<Point>& points);                                                      //sortowanie punktów wokół środka ciężkości
    static Mat alphaBlend(const Mat &src1, const Mat& src2, float alpha);                               //mieszanie dwóch obrazów z podanym współczynnikiem przenikania
    static void alphaBlendMask(Mat &dst, Mat &intensity_mask, Scalar color, Point offset, float alpha); //rysowanie maski intensywności o podanym kolorze, przesunięciu oraz współczynniku przenikania na danym obrazie
};

#endif // CONTOUR_H
