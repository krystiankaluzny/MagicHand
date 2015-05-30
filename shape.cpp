#include "shape.h"
#include <cmath>
#include <map>

#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923

Shape::Shape()
{
    type = ST_NONE;
    size_signature = 100;
    erasing_time = 0;
    max_erasing_time = 3000;
    center = Point(-1, -1);
}

Shape::Shape(vector<Point> &draw_contour) : Shape()
{
    identifyAndApproximation(draw_contour);
    createMask();
}

bool Shape::isValid()
{
    return type != ST_NONE;
}

void Shape::drawShape(Mat &out, int dt)
{
    Scalar color;
    if(type == ST_CIRCLE)
        color = Scalar(255, 0, 0);
    else if(type == ST_RECTANGLE)
        color = Scalar(0, 255, 0);
    else if(type == ST_TRIANGLE)
        color = Scalar(0, 255, 255);

    float erasing_progress = (float)erasing_time / max_erasing_time;

    color = Scalar(color[0] * (1.0 - erasing_progress), color[1] * (1.0 - erasing_progress), color[2] + erasing_progress * (255 - color[2]));
    Contour::alphaBlendMask(out, intensity_mask, color, offset, 0.5 * (1.0 + erasing_progress));

    erasing_time -= dt;
    if(erasing_time < 0) erasing_time = 0;
}

void Shape::moveShapeTo(Point &point)
{
    int dx = point.x - center.x;
    int dy = point.y - center.y;
    if(dx > 130 || dy > 130) return; //unikamy zbyt dużych nagłuch przeskoków

    center.x += dx;
    center.y += dy;

    offset.x += dx;
    offset.y += dy;
}

bool Shape::removeShape(vector<Point> &pointers, int dt)
{
    bool removing = true;
    vector<Point2f> tmp;
    for(Point &sc : shape_contour) tmp.push_back(Point2f(sc.x + offset.x, sc.y + offset.y));
    for(Point &p : pointers)
        if(pointPolygonTest(tmp, p, false) < 0)
        {
            removing = false;
            break;
        }

    if(removing)
    {
        erasing_time += 2 * dt;
        if(erasing_time > max_erasing_time)
        {
            erasing_time = max_erasing_time;
            type = ST_NONE;
        }
    }
    return removing;
}

vector<double> Shape::shapeSignature(vector<Point> &contour, double* maximum, int *index_of_max)
{
    vector<double> signature(size_signature);
    for(double& d : signature) d = 0;
    if(contour.size() == 0) return signature;

    Point c = Contour::contourCenter(contour);
    double a;
    double x, y;
    double d;
    int i;

    for(Point& p : contour)
    {
        x = p.x - c.x;
        y = p.y - c.y;
        if(fabs(x) < 0.00001) a = M_PI_2;
        else a = atan(fabs(y/x));
        if(x >= 0 && y <= 0) a = M_PI - a;
        else if(x >= 0 && y >= 0) a = M_PI + a;
        else if(x <= 0 && y > 0) a = 2 * M_PI - a;

        d = sqrt(x*x + y*y);
        i = a / 2.0 / M_PI * size_signature;
        if(signature[i] == 0) signature[i] = d;
        else signature[i] = (signature[i] + d) / 2;
    }
    //znajdowanie indeksu maksimum
    int index_max = 0;
    double max = signature[0];
    for(int i = 1; i < size_signature; i++)
        if(signature[i] > max)
        {
            max = signature[i];
            index_max = i;
        }
    if(maximum != nullptr) *maximum = max;
    if(index_of_max != nullptr) *index_of_max = index_max;

    if(max < 0.00001) max = 1; //unikamy dzielenie przez 0

    //przesunięcie cykliczne tak aby maksimum było na początku oraz normalizacja do [0; 1]
    vector<double> tmp;
    for(int i = 0; i < index_max; i++)
        tmp.push_back(signature[i]);

    for(int i = 0; i + index_max < size_signature; i++)
        signature[i] = signature[i+index_max] / max;

    for(int i = size_signature - index_max; i < size_signature; i++)
        signature[i] = tmp[i - size_signature + index_max] / max;

    //usuwanie zer przez aproksymację liniową
    int index1 = 0, index2 = 0;
    double delta = 0;
    for(int i = 0; i < size_signature; i++)
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
    if(index1 != index2)
    {
        delta = (signature[0] - signature[index1]) / (index2 - index1 + 1);
        for(int j = index1 + 1; j <= index2; j++)
            signature[j] = signature[j-1] + delta;
    }

    return signature;
}

void Shape::identifyAndApproximation(vector<Point> &draw_contour)
{
    if(draw_contour.size() < 2) return;

    //=================== wzorcowe kształty ===================
    vector<Point> tmp_contours;
    multimap<ShapeType,  vector<Point> > reference_shapes;

    //prostokąt
    tmp_contours.clear();
    tmp_contours.push_back(Point(-150, 100));
    tmp_contours.push_back(Point(-150, -100));
    tmp_contours.push_back(Point(150, -100));
    tmp_contours.push_back(Point(150, 100));
    reference_shapes.insert(make_pair(ST_RECTANGLE, tmp_contours));
    tmp_contours.clear();
    tmp_contours.push_back(Point(-100, -150));
    tmp_contours.push_back(Point(100, -150));
    tmp_contours.push_back(Point(100, 150));
    tmp_contours.push_back(Point(-100, 150));
    reference_shapes.insert(make_pair(ST_RECTANGLE, tmp_contours));
    tmp_contours.clear();
    tmp_contours.push_back(Point(-250, 100));
    tmp_contours.push_back(Point(-250, -100));
    tmp_contours.push_back(Point(250, -100));
    tmp_contours.push_back(Point(250, 100));
    reference_shapes.insert(make_pair(ST_RECTANGLE, tmp_contours));
    tmp_contours.clear();
    tmp_contours.push_back(Point(-100, -250));
    tmp_contours.push_back(Point(100, -250));
    tmp_contours.push_back(Point(100, 250));
    tmp_contours.push_back(Point(-100, 250));
    reference_shapes.insert(make_pair(ST_RECTANGLE, tmp_contours));
    tmp_contours.clear();
    tmp_contours.push_back(Point(-100, 0));
    tmp_contours.push_back(Point(0, 100));
    tmp_contours.push_back(Point(100, 0));
    tmp_contours.push_back(Point(0, -100));
    reference_shapes.insert(make_pair(ST_RECTANGLE, tmp_contours));

    //trójkąt
    tmp_contours.clear();
    tmp_contours.push_back(Point(0, 1000));
    tmp_contours.push_back(Point(577, 0));
    tmp_contours.push_back(Point(-577, 0));
    reference_shapes.insert(make_pair(ST_TRIANGLE, tmp_contours));
    tmp_contours.clear();
    tmp_contours.push_back(Point(0, 0));
    tmp_contours.push_back(Point(1000, 0));
    tmp_contours.push_back(Point(0, 1000));
    reference_shapes.insert(make_pair(ST_TRIANGLE, tmp_contours));
    tmp_contours.clear();
    tmp_contours.push_back(Point(0, 0));
    tmp_contours.push_back(Point(0, 1000));
    tmp_contours.push_back(Point(1000, 0));
    reference_shapes.insert(make_pair(ST_TRIANGLE, tmp_contours));

    //=================== sygnatura konturu wejściowego ===================
    double max = 0, norm_min = 1.0;
    int index_of_maximum; //index maksimum PRZED cyklicznym przesunięciem w wektorze sygnatury w shapeSignature
    int index_of_minimum; //index minimum PO cyklicznym przesunięciem w wektorze sygnatury w shapeSignature (względem indeksu maksimum)
    Contour::increaseContourPrecision(draw_contour, size_signature);
    vector<double> signature = shapeSignature(draw_contour, &max, &index_of_maximum);

    for(int i = 0; i < signature.size(); i++)
        if(signature[i] < norm_min)
        {
            norm_min = signature[i];
            index_of_minimum = i;
        }
    if(index_of_minimum > size_signature / 2) index_of_minimum -= size_signature / 2;

    //=================== rozpoznawanie typu ===================
    vector<double> reference_signature;
    double pearson_coefficient = 0.0, max_pearson_coefficient = 0.0;
    multimap<ShapeType, vector<Point> >::iterator iter;

    for(iter = reference_shapes.begin(); iter != reference_shapes.end(); iter++)
    {
        Contour::increaseContourPrecision((*iter).second, size_signature);
        reference_signature = shapeSignature((*iter).second);
        pearson_coefficient = Contour::pearsonCoefficient(signature, reference_signature);
        if(pearson_coefficient > 0.6 && pearson_coefficient > max_pearson_coefficient)
        {
            max_pearson_coefficient = pearson_coefficient;
            type = static_cast<ShapeType>((*iter).first);
        }
    }
    double malinowska_coefficient = Contour::malinowskaCoefficient(draw_contour);
    if((max_pearson_coefficient < 0.80 && malinowska_coefficient < 0.1) || malinowska_coefficient < 0.03) type = ST_CIRCLE;

     //----------------------------------------
    cout << "type przed" << " " << (type == ST_CIRCLE ? "ST_CIRCLE" : (type == ST_RECTANGLE ? "ST_RECTANGLE" : (type == ST_TRIANGLE ? "ST_TRIANGLE" : "ST_NONE")))  << " " << max_pearson_coefficient << endl;
    cout << "Malinowska " << Contour::malinowskaCoefficient(draw_contour) << " " << norm_min << endl;
     //----------------------------------------

    //=================== aproksymacja figury ===================
    if(type != ST_NONE)
    {
        center = Contour::contourCenter(draw_contour);

        double angle, angle2, area_ratio;
        if(type == ST_RECTANGLE)
        {
            int resize_counter = 0;

            if(index_of_minimum > size_signature / 4)
                angle2 = (asin(min(contourArea(draw_contour) / (2 * max * max), 0.99)) + 4.0 * M_PI * (index_of_minimum - size_signature / 4) / size_signature) / 2;
            else
                angle2 = (M_PI - asin(min(contourArea(draw_contour) / (2 * max * max), 0.99)) + 4.0 * M_PI * (index_of_minimum) / size_signature) / 2;

            while(true)
            {
                shape_contour.clear();
                angle = 2.0 * M_PI * (index_of_maximum) / size_signature;
                shape_contour.push_back(Point(-cos(angle) * max * 0.92 + center.x,
                                              -sin(angle) * max * 0.92 + center.y));
                angle = 2.0 * M_PI * (index_of_maximum) / size_signature + angle2;
                shape_contour.push_back(Point(-cos(angle) * max * 0.92 + center.x,
                                              -sin(angle) * max * 0.92 + center.y));
                angle = 2.0 * M_PI * (index_of_maximum) / size_signature + M_PI;
                shape_contour.push_back(Point(-cos(angle) * max * 0.92 + center.x,
                                              -sin(angle) * max * 0.92 + center.y));
                angle = 2.0 * M_PI * (index_of_maximum) / size_signature + angle2 + M_PI;
                shape_contour.push_back(Point(-cos(angle) * max * 0.92 + center.x,
                                              -sin(angle) * max * 0.92 + center.y));

                area_ratio = contourArea(shape_contour) / contourArea(draw_contour);
                if(area_ratio < 0.8) //prostokąt jest za mały
                {
                    cout << "za mały " << angle2 << endl;
                    if(angle2 < M_PI_2) angle2 *= 1.05;
                    else angle2 /= 1.05;
                    ++resize_counter;
                    if(resize_counter > 30) break;
                }
                else if(area_ratio > 1.2) //prostokąt jest za duży
                {
                    cout << "za duzy " << angle2 << endl;
                    if(angle2 < M_PI_2) angle2 /= 1.05;
                    else angle2 *= 1.05;
                    ++resize_counter;
                    if(resize_counter > 30) break;
                }
                else break;
            }
        }
        else if(type == ST_TRIANGLE)
        {
            double max1 = 0, max2 = 0;
            int index_max1 = 0, index_max2 = 0;
            double a = signature[0], b = signature[size_signature / 2], c;

            double d2_1, d2_2;
            for(int i = 1; i < size_signature / 2; ++i)
            {
                c = signature[i];
                d2_1 = sqrt(a * a + c * c - 2 * a * c * cos(2.0 * M_PI * i / size_signature));
                d2_2 = sqrt(b * b + c * c - 2 * b * c * cos(M_PI - 2.0 * M_PI * i / size_signature));
                if(d2_1 + d2_2 > max1)
                {
                    max1 = d2_1 + d2_2;
                    index_max1 = i;
                }
            }

            for(int i = size_signature / 2 + 1; i < size_signature; ++i)
            {
                c = signature[i];
                d2_1 = sqrt(a * a + c * c - 2 * a * c * cos(2.0 * M_PI * i / size_signature));
                d2_2 = sqrt(b * b + c * c - 2 * b * c * cos(M_PI - 2.0 * M_PI * i / size_signature));
                if(d2_1 + d2_2 > max2)
                {
                    max2 = d2_1 + d2_2;
                    index_max2 = i;
                }
            }

            angle = 2.0 * M_PI * index_of_maximum / size_signature;
            shape_contour.push_back(Point(-cos(angle) * signature[0] * max * 0.98 + center.x,
                                          -sin(angle) * signature[0] * max * 0.98 + center.y));
            angle = 2.0 * M_PI * (index_of_maximum + index_max1) / size_signature;
            shape_contour.push_back(Point(-cos(angle) * signature[index_max1] * max * 0.98 + center.x,
                                          -sin(angle) * signature[index_max1] * max * 0.98 + center.y));
            angle = 2.0 * M_PI * (index_of_maximum + index_max2) / size_signature;
            shape_contour.push_back(Point(-cos(angle) * signature[index_max2] * max * 0.98 + center.x,
                                          -sin(angle) * signature[index_max2] * max * 0.98 + center.y));
        }
        else if(type == ST_CIRCLE)
        {
            double r = (norm_min + signature[0]) / 2 * max;
            for(int i = 0; i < 40; i++)
                shape_contour.push_back(Point(r * cos(2.0 * M_PI * i / 40) + center.x,
                                              r * sin(2.0 * M_PI * i / 40) + center.y));
        }
    }
    cout << "type PO" << " " << (type == ST_CIRCLE ? "ST_CIRCLE" : (type == ST_RECTANGLE ? "ST_RECTANGLE" : (type == ST_TRIANGLE ? "ST_TRIANGLE" : "ST_NONE")))  << " " << max_pearson_coefficient << endl;
}

void Shape::createMask()
{
    if(type == ST_NONE) return;

    center = Contour::contourCenter(shape_contour);
    Rect br = boundingRect(shape_contour);
    offset.x = br.x;
    offset.y = br.y;
    for(Point& p : shape_contour)
    {
        p.x -= offset.x;
        p.y -= offset.y;
    }

    intensity_mask = Mat(br.size(), CV_8UC1);
    intensity_mask.setTo(0);
    fillPoly(intensity_mask, vector< vector<Point> >(1, shape_contour), 255);
    polylines(intensity_mask, vector< vector<Point> >(1, shape_contour), true, 215, 2);
    if(center != Point(-1, -1))
        circle(intensity_mask, Point(center.x - br.x, center.y - br.y), 10, 215, -1);
}
