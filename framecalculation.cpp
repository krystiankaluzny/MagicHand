#include "framecalculation.h"
#include <cmath>
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923

FrameCalculation::FrameCalculation(string window_name)
{
    this->window_name = window_name;
    average_color = Vec3i(60, 100, 100);
    lower_color = Vec3i(40, 50, 50);
    upper_color = Vec3i(80, 150, 150);
    rgb_color = Vec3i(0, 255, 0);

    calibrated = false;
    drawing = false;
    identified = false;
    blink_counter = 0;
    calibration_windows_size = 50;
    calibration_counter = 0;
    min_countur_size = 20;

    morphology_element = Mat(3, 3, CV_8UC1, Scalar(0,0,0));
    circle( morphology_element, Point( morphology_element.cols/2, morphology_element.rows/2), morphology_element.rows/2, 255, -1);
}

FrameCalculation::~FrameCalculation()
{
    for(auto s : shapes) delete s;
}

void FrameCalculation::calculate(Mat &frame)
{
    if(calibrated)
    {
        frame.copyTo(output);
        selecPointers(frame); //szukanie konturów wskaźników

        if(pointers_contours.size() == 1)//rysowanie jeśli jeden wskaźnik
        {
            if(blink_counter > 0) blink_counter--; //czekamy przez chwilę zanim zaczniemy rysować
            if(blink_counter == 0 && !drawing) //zaczynamy rysowanie
            {
                drawing = true;
                draw_points.clear();
            }
            if(drawing) //dodajemy kolejne punkty
            {
                draw_points.push_back(FrameCalculation::contourCenter(pointers_contours.at(0)));
                identified = false;
                if(draw_points.size() > min_countur_size)
                {
                    double x, y, d;
                    for(int i = 0; i < min_countur_size / 2; i++) //sprawdzamy czy zamkneliśmy rysowaną figurę
                    {
                        x = draw_points[draw_points.size() - 1].x - draw_points[i].x;
                        y = draw_points[draw_points.size() - 1].y - draw_points[i].y;
                        d = sqrt(x*x + y*y);
                        //jeśli koniec zetkną się z początkiem o ile nie staliśmy w jednym miejscu to koniec rysowanie figury
                        if(d < (min_countur_size / 2 - 1) && (contourArea(draw_points) > M_PI * min_countur_size * min_countur_size / 4))
                        {
                            blink_counter = 13;
                            drawing = false;
                        }
                    }
                }
            }
        }
//        else if(pointers_contours.size() > 1)
//        {
//            if(blink_counter > 0)
//                blink_counter--;
//        }
        else //nie ma wskaźnika
        {
            blink_counter++;
        }

        if(blink_counter > 12) //jeśli kursora nie ma przez 12 ramek i nie zamkneliśmy figury to koniec rysowania
        {
            drawing = false;
            blink_counter = 13;
            if(drawing)
                draw_points.clear();
        }


        //narysowanie wskaźników
        for(int i = 0; i < pointers_contours.size(); i++)
            drawContours(output, pointers_contours, i, Scalar(rgb_color[0], rgb_color[1], rgb_color[2]), -1);


        if(!drawing) //jeśli rysownaie się skończyło to rozpoznaj kształt
        {
            if(!identified)identifyShape(); //jeśli nie rozpoznano jeszcze narysowanego kształtu
            output = Mat::zeros(frame.rows, frame.cols, frame.type());

            drawContours(output, vector<vector<Point> >(1,draw_points), -1, Scalar(rgb_color[0], rgb_color[1], rgb_color[2]), 3);
        }
        else //podczas rysowania
        {
            drawSingleContour(output, draw_points, 3);
        }

        for(auto s : shapes)
        {
            drawSingleContour(output, s->shape_contour, 3, true);
        }
        string text = "Przedzial: (" + intToString(lower_color[0]) + "; " + intToString(lower_color[1]) + "; " + intToString(lower_color[2]) + ") - "
                +  "(" + intToString(upper_color[0]) + "; " + intToString(upper_color[1]) + "; " + intToString(upper_color[2]) + ")";
        putText(output, text, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 0, 0), 2);
    }
    else
    {
        calibration(frame);
    }
    imshow(window_name, output);
}

double FrameCalculation::malinowskaCoefficient(double L, double S)
{
    if(S < 0.00001) return 10;
    return L / (2 * sqrt(M_PI * S)) - 1;
}

void FrameCalculation::calibration(Mat &frame)
{
    frame.copyTo(output);
    Rect cr = Rect((frame.cols-calibration_windows_size)/2, (frame.rows-calibration_windows_size)/2,
                   calibration_windows_size, calibration_windows_size);
    Mat cm = Mat(frame,cr);
    cvtColor(cm, hsv, CV_RGB2HSV);
    Scalar tmp_sum = sum(hsv);

    average_color = Vec3i(tmp_sum[0]/hsv.total(), tmp_sum[1]/hsv.total(), tmp_sum[2]/hsv.total()); //średni kolor hsv
    lower_color = Vec3i((average_color[0] - 20 + lower_color[0]) / 2, (average_color[1] - 100 + lower_color[1])/2, (average_color[2] - 100 + lower_color[2])/2);
    upper_color = Vec3i((average_color[0] + 20 + upper_color[0]) / 2, 240, 240);

    if(lower_color[0] < 0) lower_color[0] = 0;
    if(lower_color[1] < 0) lower_color[1] = 0;
    if(lower_color[2] < 0) lower_color[2] = 0;

    if(upper_color[0] > 180) upper_color[0] = 180;
    if(upper_color[1] > 255) upper_color[1] = 255;
    if(upper_color[2] > 255) upper_color[2] = 255;

    rgb_color = HSVToRGB(average_color);

//    cvtColor(frame, hsv, CV_BGR2HSV);
    inRange(hsv, lower_color, upper_color, binary);

    morphologyEx(binary, binary, MORPH_CLOSE, morphology_element, Point(-1, -1), 1);
    double area_sum = countMaskPixels(binary);
    findContours(binary, contours, CV_RETR_LIST , CV_CHAIN_APPROX_NONE);
    for(int i = 0; i < contours.size(); i++)
    {
        drawContours(output, contours, i, Scalar(rgb_color[0], rgb_color[1], rgb_color[2]), -1, 8, noArray(), 2147483647,
                Point((frame.cols-calibration_windows_size)/2, (frame.rows-calibration_windows_size)/2));
    }
    if(area_sum / cm.total() > 0.95)
    {
        calibration_counter++;
        if(calibration_counter > 100) calibrated = true;
    }
    else if(calibration_counter > 0) calibration_counter--;

    rectangle(output, cr, Scalar(255, 0, 0), 2);
    string text = "Przedzial: (" + intToString(lower_color[0]) + "; " + intToString(lower_color[1]) + "; " + intToString(lower_color[2]) + ") - "
            +  "(" + intToString(upper_color[0]) + "; " + intToString(upper_color[1]) + "; " + intToString(upper_color[2]) + ")";
    putText(output, text, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 0, 0), 2);
    text = "Wypelnienie: " + intToString(area_sum / cm.total() * 100) + "%";
    putText(output, text, Point(10, 40), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 0, 0), 2);
    text = "Kalibracja: " + intToString(calibration_counter) + "%";
    putText(output, text, Point(10, 60), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 0, 0), 2);
}

void FrameCalculation::selecPointers(Mat &frame)
{
    pointers_contours.clear();
    cvtColor(frame, hsv, CV_RGB2HSV);
    inRange(hsv, lower_color, upper_color, binary);
    morphologyEx(binary, binary, MORPH_CLOSE, morphology_element, Point(-1, -1), 1);
    findContours(binary, contours, CV_RETR_LIST , CV_CHAIN_APPROX_NONE);
    double area, length;
    double mc = 0;
    for(int i = 0; i < contours.size(); i++)
    {
        area = contourArea(contours[i]);
        length = arcLength(contours[i], true);
        mc = malinowskaCoefficient(length, area);
        if( mc < 0.35 && area > 530)
        {
            pointers_contours.push_back(contours[i]);
        }
    }
}

void FrameCalculation::identifyShape()
{
    identified = true;
    if(draw_points.size() < min_countur_size) return;
    vector<Point> draw_contour;
    approxPolyDP(draw_points, draw_contour, 2, true);
//    convexHull( Mat(draw_points), draw_contour, false ); //otoczka wypukła //TODO sprawdzić czy potrzebne

    cout << "Size " << draw_points.size() << endl;
    Shape* shape  = new Shape(draw_contour);
    if(shape->isValid())
    {
        shapes.push_back(shape);
        //TODO dodaj shape do jakiegoś kontenera obiektów
    }
}

string FrameCalculation::intToString(int num)
{
    if(num == 0) return "0";

    std::string buf;
    bool negative = false;
    if(num < 0)
    {
        negative = true;
        num = -num;
    }
    for(; num; num /= 10) buf = "0123456789abcdef"[num % 10] + buf;
    if(negative) buf = "-" + buf;
    return buf;
}

Vec3i FrameCalculation::HSVToRGB(Vec3i& hsv)
{
    Mat RGB;
    Mat HSV(1, 1, CV_8UC3, Scalar(hsv[0], hsv[1], hsv[2]));
    cvtColor(HSV, RGB,CV_HSV2RGB);
    return RGB.at<Vec3b>(0,0);
}

int FrameCalculation::countMaskPixels(Mat &mask)
{
    int sum = 0;
    for(int i = 0; i < mask.cols; i++)
        for(int j = 0; j < mask.rows; j++)
            if(mask.at<unsigned char>(i, j) > 0) sum++;
    return sum;
}

Point FrameCalculation::contourCenter(vector<Point> &contour)
{
    Moments m = moments(contour, false);
    return Point( m.m10 / m.m00 , m.m01 / m.m00 );
}

void FrameCalculation::drawSingleContour(Mat &out, vector<Point> &contour, int size, bool close)
{
    polylines(out, vector<vector<Point> >(1,contour), close, Scalar(rgb_color[0], 255, rgb_color[2]), size);
}
