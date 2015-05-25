#include "shape.h"
#include <cmath>
#include <map>

#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923

Shape::Shape()
{
    type = ST_NONE;
    m_size_signature = 100;
}

Shape::Shape(vector<Point> &draw_contour)
{
    type = ST_NONE;
    m_size_signature = 100;

    identifyShape(draw_contour);
}

bool Shape::isValid()
{
    return type != ST_NONE;
}

void Shape::draw(Mat &out)
{
    Scalar color;
    if(type == ST_CIRCLE)
        color = Scalar(255, 0, 0);
    else if(type == ST_RECTANGLE)
        color = Scalar(0, 255, 0);
    else if(type == ST_TRIANGLE)
        color = Scalar(0, 0, 255);
    Contour::drawPoly(out, shape_contour, center, color);
}

void Shape::moveTo(Point &point)
{
    int dx = point.x - center.x;
    int dy = point.y - center.y;
    if(dx > 130 || dy > 130) return;
    for(Point& p : shape_contour)
    {
        p.x += dx;
        p.y += dy;
    }
    center.x += dx;
    center.y += dy;
}

vector<double> Shape::shapeSignature(vector<Point> &contour, double* maximum, int *index_of_max)
{
    vector<double> signature(m_size_signature);
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
        i = a / 2.0 / M_PI * m_size_signature;
        if(signature[i] == 0) signature[i] = d;
        else signature[i] = (signature[i] + d) / 2;
    }
    //znajdowanie indeksu maksimum
    int index_max = 0;
    double max = signature[0];
    for(int i = 1; i < m_size_signature; i++)
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

    for(int i = 0; i + index_max < m_size_signature; i++)
        signature[i] = signature[i+index_max] / max;

    for(int i = m_size_signature - index_max; i < m_size_signature; i++)
        signature[i] = tmp[i - m_size_signature + index_max] / max;

    //usuwanie zer przez aproksymację liniową
    int index1 = 0, index2 = 0;
    double delta = 0;
    for(int i = 0; i < m_size_signature; i++)
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

void Shape::identifyShape(vector<Point> &draw_contour)
{
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
    int index_of_maximum; //index maksimum przed cyklicznym przesunięciem w wektorze sygnatury w shapeSignature
    Contour::increaseContourPrecision(draw_contour, m_size_signature);
    vector<double> signature = shapeSignature(draw_contour, &max, &index_of_maximum);

    for(int i = 0; i < signature.size(); i++) if(signature[i] < norm_min) norm_min = signature[i]; //znalezienie minimum sygnatury
    //=================== rozpoznawanie typu ===================
    vector<double> reference_signature;
    double pearson_coefficient = 0.0, max_pearson_coefficient = 0.0;
    multimap<ShapeType, vector<Point> >::iterator iter;

    //----------------------------------------
    Mat t = Mat::zeros(500, 250, CV_8SC3);
    int k = -1;
    //----------------------------------------

    for(iter = reference_shapes.begin(); iter != reference_shapes.end(); iter++)
    {
        k++;
        Contour::increaseContourPrecision((*iter).second, m_size_signature);
        reference_signature = shapeSignature((*iter).second);
        pearson_coefficient = Contour::pearsonCoefficient(signature, reference_signature);
        if(pearson_coefficient > 0.5 && pearson_coefficient > max_pearson_coefficient)
        {
            max_pearson_coefficient = pearson_coefficient;
            type = static_cast<ShapeType>((*iter).first);
        }

        //----------------------------------------
        tmp_contours.clear();
        for(int i = 0; i < reference_signature.size(); i++)
            tmp_contours.push_back(Point(i + reference_signature.size(), reference_signature[i] * 80 + k * 40));
        polylines(t, vector<vector<Point> >(1,tmp_contours), false, Scalar(rand()%255, rand()%255, rand()%255), 1);
        //----------------------------------------
    }

    if(Contour::malinowskaCoefficient(draw_contour) < 0.1 && max_pearson_coefficient < 0.8) type = ST_CIRCLE;

     //----------------------------------------
    cout << "type przed" << " " << (type == ST_CIRCLE ? "ST_CIRCLE" : (type == ST_RECTANGLE ? "ST_RECTANGLE" : (type == ST_TRIANGLE ? "ST_TRIANGLE" : "ST_NONE")))  << " " << max_pearson_coefficient << endl;
    cout << "Malinowska " << Contour::malinowskaCoefficient(draw_contour) << " " << norm_min << endl;
     //----------------------------------------

    //=================== aproksymacja figury ===================
    vector<int> pick_index;
    if(type != ST_NONE)
    {
        center = Contour::contourCenter(draw_contour); //chwilowy środek figury
        pick_index.push_back(0);

        double poch1 = 0, poch2 = 0;


        for(int i = 1; i < signature.size() - 1; i++)
        {
            poch2 = signature[i] - signature[i-1];

            if(poch1 > 0 && poch2 < 0 && signature[i] > norm_min * 1.2 && i - pick_index[pick_index.size()-1] > 5)
            {
                cout << poch1 * poch2 * 1000 << endl;
                pick_index.push_back(i);
            }
            poch1 = poch2;
        }


        double angle;
        if(type == ST_RECTANGLE)
        {
            cout << "REC 1"<<endl;
            if(pick_index.size() < 3 || pick_index.size() > 7) type = ST_NONE;
            else if((pick_index.size() == 3 || pick_index.size() == 7) && max_pearson_coefficient < 0.7) type = ST_NONE;
            else
            {
                int tmp_index;
                angle = 2.0 * M_PI * (index_of_maximum) / m_size_signature;
                shape_contour.push_back(Point(-cos(angle) * max * 0.92 + center.x,
                                              -sin(angle) * max * 0.92 + center.y));
                angle = 2.0 * M_PI * (pick_index[1] + index_of_maximum) / m_size_signature;
                shape_contour.push_back(Point(-cos(angle) * max * 0.92 + center.x,
                                              -sin(angle) * max * 0.92 + center.y));
                angle = 2.0 * M_PI * (index_of_maximum) / m_size_signature + M_PI;
                shape_contour.push_back(Point(-cos(angle) * max * 0.92 + center.x,
                                              -sin(angle) * max * 0.92 + center.y));
                angle = 2.0 * M_PI * (pick_index[1] + index_of_maximum) / m_size_signature + M_PI;
                shape_contour.push_back(Point(-cos(angle) * max * 0.92 + center.x,
                                              -sin(angle) * max * 0.92 + center.y));
            }


        }
        else if(type == ST_TRIANGLE)
        {

            if(pick_index.size() < 3 || pick_index.size() > 5) type = ST_NONE;
            angle = 2.0 * M_PI * index_of_maximum / m_size_signature;
            shape_contour.push_back(Point(-cos(angle) * signature[pick_index[0]] * max * 0.98 + center.x,
                                          -sin(angle) * signature[pick_index[0]] * max * 0.98 + center.y));
            int last_index = 0;
            for(int i = 1; i < pick_index.size(); i++)
            {
                if((pick_index[i] - pick_index[last_index] > 50.0 / 360.0 * m_size_signature) && (pick_index[i] - pick_index[last_index] < 170.0 / 360.0 * m_size_signature))
                {
                    angle = 2.0 * M_PI * (pick_index[i] + index_of_maximum) / m_size_signature;
                    shape_contour.push_back(Point(-cos(angle) * signature[pick_index[i]] * max * 0.98 + center.x,
                                                  -sin(angle) * signature[pick_index[i]] * max * 0.98 + center.y));
                    last_index = i;
                }
            }

        }
        else if(type == ST_CIRCLE)
        {
            double r = (norm_min + signature[0]) / 2 * max;
            for(int i = 0; i < 40; i++)
                shape_contour.push_back(Point(r * cos(2.0 * M_PI * i / 40) + center.x,
                                              r * sin(2.0 * M_PI * i / 40) + center.y));
        }

        center = Contour::contourCenter(shape_contour);
    }

    cout << "DDDDD 4"<<endl;
    cout << "type po" << " " << (type == ST_CIRCLE ? "ST_CIRCLE" : (type == ST_RECTANGLE ? "ST_RECTANGLE" : (type == ST_TRIANGLE ? "ST_TRIANGLE" : "ST_NONE")))  << " " << max_pearson_coefficient << endl;

    //=================== Rysowanie sygnatury ===================

    for(int i = 0; i < reference_shapes.size(); i++)
    {
        tmp_contours.clear();
        for(int i = 0; i < signature.size(); i++)
            tmp_contours.push_back(Point(i, signature[i] * 100));
        polylines(t, vector<vector<Point> >(1,tmp_contours), false, Scalar(255, 0, 0), 1);

    }
    for(int& a : pick_index)
    {
        tmp_contours.clear();
        tmp_contours.push_back(Point(a, 0));
        tmp_contours.push_back(Point(a, 100));
        polylines(t, vector<vector<Point> >(1,tmp_contours), false, Scalar(255, 0, 255), 1);
    }
    cout << "DDDDD 5"<<endl;
    //        tmp_contours.clear();
    //        for(int i = 0; i < reference_signature.size(); i++)
    //            tmp_contours.push_back(Point(i, reference_signature[i] * 100));
    //        polylines(t, vector<vector<Point> >(1,tmp_contours), false, Scalar(rand()%255, rand()%255, rand()%255), 1);

//    tmp_contours.clear();
//    for(int i = 0; i < triangle_signature.size(); i++)
//        tmp_contours.push_back(Point(i, triangle_signature[i] * 100));
//    polylines(t, vector<vector<Point> >(1,tmp_contours), false, Scalar(0, 255, 0), 1);

//    tmp_contours.clear();
//    for(int i = 0; i < signature.size(); i++)
//        tmp_contours.push_back(Point(i, signature[i] * 100));
//    polylines(t, vector<vector<Point> >(1,tmp_contours), false, Scalar(0, 0, 255), 1);


    imshow("Sygnatura", t);
}

