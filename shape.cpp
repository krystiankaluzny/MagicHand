#include "shape.h"
#include <cmath>
#include <map>

#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
Shape::Shape()
{
    m_type = ST_NONE;
    m_size_signature = 100;
}

Shape::Shape(vector<Point> &draw_contour)
{
    m_type = ST_NONE;
    m_size_signature = 100;

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
    tmp_contours.push_back(Point(-100, 0));
    tmp_contours.push_back(Point(0, 100));
    tmp_contours.push_back(Point(100, 0));
    tmp_contours.push_back(Point(0, -100));
    reference_shapes.insert(make_pair(ST_RECTANGLE, tmp_contours));

    //koło
    tmp_contours.clear();
    reference_shapes.insert(make_pair(ST_CIRCLE, tmp_contours));;

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
    int index_of_maximum; //index maksimum przed rotacją wektora sygnatury w shapeSignature
    increaseContourPrecision(draw_contour);
    vector<double> signature = shapeSignature(draw_contour, &max, &index_of_maximum);
    for(double& s : signature)  if(s < norm_min) norm_min = s;

    //=================== rozpoznawanie typu ===================
    vector<double> reference_signature;
    double pearson_coefficient = 0.0, max_pearson_coefficient = 0.0;
    multimap<ShapeType, vector<Point> >::iterator iter;
    if(Shape::malinowskaCoefficient(draw_contour) > 0.1)
        for(iter = reference_shapes.begin(); iter != reference_shapes.end(); iter++)
        {
            increaseContourPrecision((*iter).second);
            reference_signature = shapeSignature((*iter).second);
            pearson_coefficient = pearsonCoefficient(signature, reference_signature);
            if(pearson_coefficient > 0.5 && pearson_coefficient > max_pearson_coefficient)
            {
                max_pearson_coefficient = pearson_coefficient;
                m_type = static_cast<ShapeType>((*iter).first);
            }
        }
    else m_type = ST_CIRCLE;

//    cout << m_type << " " << pearson_coefficient << " " << max_pearson_coefficient << endl;
        cout << "Malinowska " << Shape::malinowskaCoefficient(draw_contour) << " " << norm_min << endl;

    //=================== aproksymacja figury ===================
    vector<int> pick_index;
    if(m_type != ST_NONE)
    {
        center = Shape::contourCenter(draw_contour);
        pick_index.push_back(0);

        int poch1 = 0, poch2 = 0;
        for(int i = 1; i < signature.size() - 1; i++)
        {
            if(signature[i-1] < signature[i]) poch2 = 1;
            else if(signature[i-1] > signature[i]) poch2 = -1;
            else poch2 = 0;

            if(poch1 == 1 && poch2 == -1 && signature[i] > norm_min * 1.2 ) pick_index.push_back(i);
            poch1 = poch2;
        }

//        cout << "DUPA " << index_of_maximum <<endl;
//        for(int& a : pick_index)
//            cout << a << endl;
//            cout << 2.0 * M_PI * (a + index_of_maximum) / m_size_signature << endl;
        double angle;
        if(m_type == ST_RECTANGLE)
        {
            if(pick_index.size() < 4 || pick_index.size() > 6) m_type = ST_NONE;
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
        else if(m_type == ST_TRIANGLE)
        {
            if(pick_index.size() < 3 || pick_index.size() > 5) m_type = ST_NONE;
            angle = 2.0 * M_PI * index_of_maximum / m_size_signature;
            shape_contour.push_back(Point(-cos(angle) * signature[pick_index[0]] * max * 0.98 + center.x,
                                          -sin(angle) * signature[pick_index[0]] * max * 0.98 + center.y));
            int last_index = 0;
            for(int i = 1; i < pick_index.size(); i++)
            {
                if((pick_index[i] - pick_index[last_index] > 40.0 / 360.0 * m_size_signature) && (pick_index[i] - pick_index[last_index] < 170.0 / 360.0 * m_size_signature))
                {
                    angle = 2.0 * M_PI * (pick_index[i] + index_of_maximum) / m_size_signature;
                    shape_contour.push_back(Point(-cos(angle) * signature[pick_index[i]] * max * 0.98 + center.x,
                                                  -sin(angle) * signature[pick_index[i]] * max * 0.98 + center.y));
                    last_index = i;
                }
            }
        }
        else if(m_type == ST_CIRCLE)
        {
            double r = (norm_min + signature[0]) / 2 * max;
            for(int i = 0; i < 40; i++)
                shape_contour.push_back(Point(r * cos(2.0 * M_PI * i / 40) + center.x,
                                              r * sin(2.0 * M_PI * i / 40) + center.y));
        }
    }


    //=================== Rysowanie sygnatury ===================
    Mat t = Mat::zeros(300, 300, CV_8SC3);
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

bool Shape::isValid()
{
    return m_type != ST_NONE;
}

double Shape::malinowskaCoefficient(vector<Point> &contours)
{
    double S = contourArea(contours), L = arcLength(contours, true);
    if(S < 0.00001) return 10;
    return L / (2 * sqrt(M_PI * S)) - 1;
}

Point Shape::contourCenter(vector<Point> &contour)
{
    Moments m = moments(contour, false);
    return Point( m.m10 / m.m00 , m.m01 / m.m00 );
}

vector<double> Shape::shapeSignature(vector<Point> &contour, double* maximum, int *index_of_max)
{
    vector<double> signature(m_size_signature);
    for(double& d : signature) d = 0;
    if(contour.size() == 0) return signature;

    Point c = Shape::contourCenter(contour);
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

//        for(int i = 0; i < m_size_signature; i++)
//            signature[i] = signature[i] / max;
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

void Shape::increaseContourPrecision(vector<Point> &contour)
{
    vector<Point> tmp;
    double s;
    double dx, dy;
    if(contour.size() > 0 && contour.size() < m_size_signature)
    {
        s = m_size_signature / contour.size() * 1.2;
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

double Shape::pearsonCoefficient(vector<double> &s1, vector<double> &s2)
{
    if(s1.size() != s2.size()) return 0.0;
    if(s1.size() == 0) return 0.0;

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
