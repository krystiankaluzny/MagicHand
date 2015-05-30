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

    bool isValid();                                                 //czy figura jest poprawna, jeśli typ jest równy ST_NONE to figura jest niepoprawna
    void drawShape(Mat& out, int dt);                               //rysowanie kształtu z wykorzystaniem maski intensywności kolorów intensity_mask
    void moveShapeTo(Point& point);                                 //przeniesienie geometrycznego środka do danego punktu (zmianie ulega tylko center oraz offset)
    bool removeShape(vector<Point> &pointers, int dt);              //rozpoczęcie i kontynuacja usuwania figury

    Point center;                                                   //środek geometryczny narysowanej figury

private:
    vector<double> shapeSignature(vector<Point> &contour, double *maximum = nullptr, int *index_of_max = nullptr); //wyznaczenie sygnatury konturu jako odległości punktów od środka figury w funkcji kąta
    void identifyAndApproximation(vector<Point> &draw_contour);     //rozpoznawanie i przybliżanie kształtu narysowanie figury
    void createMask();                                              //tworzenie maski intensywności kolorów figury

    vector<Point> shape_contour;                                    //punkty konturu po aproksymacji przesunięte tak, że ich prostokąt otaczający (bounding rect) ma początek w punkcjie (0, 0)
    int signature_size;                                             //liczba punktów wyznaczanego spektrum (jego dokładność)
    int erasing_time;                                               //czas poprawnego usuwanie figury
    int max_erasing_time;                                           //czas po jakim zostanie usunięty obiekt
    ShapeType type;                                                 //rodzaj rozpoznanej figury jeśli równy ST_NONE to figura nie rozpoznana lub ma zostać usunięta
    Point offset;                                                   //odległość między lewym górynm rogiem prostokąta otaczającego figurę na ekranie a punktem (0, 0)
    Mat intensity_mask;                                             //maska mówiąca jak intensywna jest barwa danych pikseli
};

#endif // SHAPE_H
