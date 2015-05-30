#ifndef FRAMECALCULATION_H
#define FRAMECALCULATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <chrono>

#include "shape.h"
#include "contour.h"

using namespace std;
using namespace cv;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> frame_time;

class FrameCalculation
{
public:
    enum AppState
    {
        AS_CALIBRATION,
        AS_DRAWING,
        AS_MOVING,
        AS_REMOVING,
        AS_SHOWING
    };

    FrameCalculation(string window_name);
    ~FrameCalculation();

    void calculate(Mat& frame);

private:
    void calibration(Mat& frame, int dt);           //wybieranie wykrywanego koloru wskaźników
    void selecPointers(Mat& frame);         //wybranie wskaźników z ramki
    void addShape();                        //dodanie figury jeśli została zidentyfikowana

    string intToString(int num);            //konwersja liczby do tekstu
    Vec3i HSVToRGBColor(Vec3i &hsv);        //konwersja koloru HSV do RGB
    int countMaskPixels(Mat& mask);         //zliczenie niezerowych pikseli maski

    string window_name;                     //nazwa okna

    Mat output;                             //końcowy obraz wyjściowy
    Mat hsv;                                //aktualna ramka (lub jej fragment) w skali HSV
    Mat binary;                             //wybrane fragmenty obraz, dla których kolor jest między lower_color a upper_color
    Mat alpha_buffor;                       //obraz ze wszystkimi figurami z przenikaniem koloru
    Mat morphology_element;                 //element morfologiczny wykorzystywany w operacji zamykania podczas analizy ramki obrazu

    Vec3i average_color;                    //średni kolor wskaźnika w skali HSV
    Vec3i lower_color;                      //minimalny rozpoznawalny kolor wskaźnika w HSV
    Vec3i upper_color;                      //maksymalny rozpoznawalny kolor wskaźnika w HSV
    Vec3i rgb_color;                        //średni kolor wskaźnika w skali RGB

    vector<Shape*> shapes;                  //kształty obecne na ekranie

    vector< vector<Point> > pointers_contours; //kontury aktualnie widocznych wskaźników
    vector< vector<Point> > contours;       //kontury pomocnicze przy kalibrowaniu i wyznaczaniu konturów wskaźników
    vector<Point> draw_points;              //aktualnie punkty narysowane przez urzytkownika

    AppState state;                         //aktualny stan wskaźników
    bool identified;                        //czy zidentyfikowano narysowany kontur
    int calibration_windows_size;           //rozmiar okna kalibracji koloru
    int calibration_time_counter;           //czas poprawnej kalibracji
    int max_calibration_time_counter;           //czas poprawnej kalibracji
    int state_time_counter;                 //czas zmiany stanu
    int max_state_time_counter;             //maksymalna wartość state_time_counter
    int min_contour_size;                   //minimalna liczba punktów sysowanego konturu która może zostać rozpoznana jako konkretny kształt


    frame_time last_time;                   //punkt czasu analizy ostatniej klatki
    frame_time current_time;                //punkty czasu analizy obecnej klatki

};

#endif // FRAMECALCULATION_H
