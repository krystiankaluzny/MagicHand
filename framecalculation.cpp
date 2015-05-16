#include "framecalculation.h"

FrameCalculation::FrameCalculation(string window_name)
{
    this->window_name = window_name;
    average_color = Vec3i(60, 100, 100);
    lower_color = Vec3i(40, 50, 50);
    upper_color = Vec3i(80, 150, 150);
    rgb_color = Vec3i(0, 255, 0);

    calibrated = false;
    drawing = false;
    blink_counter = 0;
    calibration_windows_size = 50;
    calibration_counter = 0;
}

void FrameCalculation::calculate(Mat &frame)
{
    if(calibrated)
    {
        frame.copyTo(output);
        selectContours(frame);
//        cout << blink_counter << " " << draw_points.size() << endl;
//        if(pointers_contours.size() == 1)
//        {
//            if(blink_counter > 0 && drawing)
//                blink_counter--;
//            if(!drawing)
//            {
//                blink_counter = 0;
//                drawing = true;
//                draw_points.clear();
//            }
//            draw_points.push_back(contourCenter(pointers_contours.at(0)));
//        }
////        else if(pointers_contours.size() > 1)
////        {
////            if(blink_counter > 0)
////                blink_counter--;
////        }
//        else
//        {
//            blink_counter++;
//        }
//        if(blink_counter > 30)
//        {
//            drawing = false;
//        }
//        if(!drawing)
//        {
//            output = Mat::zeros(frame.rows, frame.cols, frame.type());
//            drawContours(output, vector<vector<Point> >(1,draw_points), -1, Scalar(rgb_color[0], rgb_color[1], rgb_color[2]));
//        }
//        for(int i = 0; i < pointers_contours.size(); i++)
//        {
//            drawContours(output, pointers_contours, i, Scalar(rgb_color[0], rgb_color[1], rgb_color[2]), -1);
//        }
//        cvtColor(frame, hsv, CV_RGB2HSV);
//        inRange(hsv, lower_color, upper_color, binary);
//        morphologyEx(binary, binary, MORPH_CLOSE, Mat(), Point(-1, -1), 1);
//        findContours(binary, contours, CV_RETR_LIST , CV_CHAIN_APPROX_NONE);
//        double area, length;
//        double mc = 0;
//        for(int i = 0; i < contours.size(); i++)
//        {
//            area = contourArea(contours[i]);
//            length = arcLength(contours[i], true);
//            mc = malinowskaCoefficient(length, area);
//            if( mc < 0.5 && area > 200)
//            {
//                cout << area << endl;
//            }
//        }
//        string text = "Przedzial: (" + intToString(lower_color[0]) + "; " + intToString(lower_color[1]) + "; " + intToString(lower_color[2]) + ") - "
//                +  "(" + intToString(upper_color[0]) + "; " + intToString(upper_color[1]) + "; " + intToString(upper_color[2]) + ")";
//        putText(output, text, Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 0, 0), 2);
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
    lower_color = Vec3i((average_color[0] - 15 + lower_color[0]) / 2, (average_color[1] - 100 + lower_color[1])/2, (average_color[2] - 100 + lower_color[2])/2);
    upper_color = Vec3i((average_color[0] + 15 + upper_color[0]) / 2, (average_color[1] + 100 + upper_color[1])/2, (average_color[2] + 100 + upper_color[2])/2);

    if(lower_color[0] < 0) lower_color[0] = 0;
    if(lower_color[1] < 0) lower_color[1] = 0;
    if(lower_color[2] < 0) lower_color[2] = 0;

    if(upper_color[0] > 180) upper_color[0] = 180;
    if(upper_color[1] > 255) upper_color[1] = 255;
    if(upper_color[2] > 255) upper_color[2] = 255;

    rgb_color = HSVToRGB(average_color);

//    cvtColor(frame, hsv, CV_BGR2HSV);
    inRange(hsv, lower_color, upper_color, binary);
    morphologyEx(binary, binary, MORPH_CLOSE, Mat(), Point(-1, -1), 1);
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
    morphologyEx(binary, binary, MORPH_CLOSE, Mat(), Point(-1, -1), 1);
    findContours(binary, contours, CV_RETR_LIST , CV_CHAIN_APPROX_NONE);
    double area, length;
    double mc = 0;
    for(int i = 0; i < contours.size(); i++)
    {
        area = contourArea(contours[i]);
        length = arcLength(contours[i], true);
        mc = malinowskaCoefficient(length, area);
        if( mc < 0.5 && area > 200)
        {
            pointers_contours.push_back(contours[i]);
            drawContours(output, contours, i, Scalar(rgb_color[0], rgb_color[1], rgb_color[2]), -1);
        }
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

Point FrameCalculation::contourCenter(vector<Point> contour)
{
    Moments m = moments(contour, false);
    return Point( m.m10 / m.m00 , m.m01 / m.m00 );
}
