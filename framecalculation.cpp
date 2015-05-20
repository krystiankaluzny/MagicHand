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
    write_to_file = false;
    blink_counter = 0;
    calibration_windows_size = 50;
    calibration_counter = 0;

    morphology_element = Mat(3, 3, CV_8UC1, Scalar(0,0,0));
    circle( morphology_element, Point( morphology_element.cols/2, morphology_element.rows/2), morphology_element.rows/2, 255, -1);
}

void FrameCalculation::calculate(Mat &frame)
{
    if(calibrated)
    {
        frame.copyTo(output);
        selectContours(frame); //wybranie konturów wskaźników
        if(pointers_contours.size() == 1)
        {
            if(blink_counter > 0 && drawing) blink_counter--;
            if(!drawing)
            {
                blink_counter = 0;
                drawing = true;
                draw_points.clear();
            }
            draw_points.push_back(contourCenter(pointers_contours.at(0)));
            identified = false;
            write_to_file = false;
        }
//        else if(pointers_contours.size() > 1)
//        {
//            if(blink_counter > 0)
//                blink_counter--;
//        }
        else
        {
            blink_counter++;
        }

        if(blink_counter > 10) //jeśli kursora nie ma przez 10 rramek to koniec rysowania
        {
            drawing = false;
        }


//        //narysowanie wskaźników
//        for(int i = 0; i < pointers_contours.size(); i++)
//            drawContours(output, pointers_contours, i, Scalar(rgb_color[0], rgb_color[1], rgb_color[2]), -1);


        if(!drawing) //jeśli rysownaie się skończyło to rozpoznaj kształt
        {
            if(!identified)identifyShape(); //jeśli nie rozpoznano jeszcze narysowanego kształtu
            output = Mat::zeros(frame.rows, frame.cols, frame.type());

            drawContours(output, vector<vector<Point> >(1,draw_contour), -1, Scalar(rgb_color[0], rgb_color[1], rgb_color[2]), 3);
        }
        else //podczas rysowania
        {
            drawSingleContour(output, draw_points, 3);
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

void FrameCalculation::selectContours(Mat &frame)
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
    approxPolyDP(draw_points, draw_contour, 5, true);
    convexHull( Mat(draw_points), draw_contour, false );

    //TODO
    //dodać wiele punktów pośrednich w otoczce wypukłej

    vector<double> signature = shapeSignature();

    //Rysowanie sygnatury
    vector<Point> tmp_contours;
    for(int i = 0; i < signature.size(); i++)
    {
        tmp_contours.push_back(Point(i, signature[i] * 100));
    }

    Mat t = Mat::zeros(300, 300, CV_8SC3);
    drawSingleContour(t, tmp_contours, 1);
    imshow("Sygnatura", t);
}

void FrameCalculation::writeToFile()
{
    write_to_file = true;
}

vector<double> FrameCalculation::shapeSignature()
{
    int size = 32;
    vector<double> signature(size);
    for(double& d : signature) d = 0;
    Point center = contourCenter(draw_contour);
    double a;
    double x, y;
    double d;
    int i;
    cout << "================" << endl;
    cout << size << endl << endl;

    for(Point& p : draw_contour)
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
    cout << "================aaaaaaaaaaaaaaa" << endl;
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
    if(max < 0.00001) max = 1;
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
    if(close)
    {
        drawContours(out, vector<vector<Point> >(1,contour), -1, Scalar(rgb_color[0], 255, rgb_color[2]), size);
    }
    else
    {

        polylines(out, vector<vector<Point> >(1,contour), false, Scalar(rgb_color[0], 255, rgb_color[2]), size);
    }
}
