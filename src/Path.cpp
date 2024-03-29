//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  Path.cpp
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#include "Path.h"
#include "Arguments.h"
#include "Process.h"
#include <cmath>
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  Path::Path
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
Path::Path(double t, double dt, double T, std::vector<double>& path)
{
    t_ = t;
    dt_ = dt;
    T_ = T;
    path_ = std::vector<double>(path);
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  Path::Path
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
Path::Path(Arguments& paras)
{
    T_ = paras.g_VAL<double>("T");
    dt_ = paras.g_VAL<double>("dt");
    path_ = std::vector<double>(*paras.g_PTR<std::vector<double> >("path")); 
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  Path::arithmeticAvg
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
double Path::arithmeticAvg()
{
    double ant = 0.0;
    double cnt = Path::T_/Path::dt_;
    for(std::vector<double>::iterator it = Path::path_.begin();it != Path::path_.end();it++)
        ant += *it;
    ant = ant/cnt;
    return ant;
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  Path::geometricAbg
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
double Path::geometricAvg()
{
    double ant = 1.0;
    double cnt = Path::T_/Path::dt_;
    for(std::vector<double>::iterator it = Path::path_.begin();it != Path::path_.end();it++)
        ant *= *it;
    ant = std::exp(std::log(ant)/cnt);
    return ant;
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  Path::break_up
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool Path::break_up(double level)
{
    for
        (
            std::vector<double>::iterator it = path_.begin();
            it+1 != path_.end();
            it++
        )
        if(*it > level && *(it+1) < level) return true;
    return false;
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  Path::break_down
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool Path::break_down(double level)
{
    for
        (
            std::vector<double>::iterator it = path_.begin();
            it+1 != path_.end();
            it++
        )
        if((*it < level) && (*(it+1) > level)) return true;
    return false;
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  Path::max
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
double Path::max()
{
    double maxx = 0.0;
    for
        (
            std::vector<double>::iterator it = Path::path_.begin();
            it != Path::path_.end();
            it++
        )
        if(*it > maxx) maxx = *it;
    return maxx;
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  Path::min
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
double Path::min()
{
    double minn = double(RAND_MAX);
    for
        (
            std::vector<double>::iterator it = Path::path_.begin();
            it != Path::path_.end();
            it++
        )
        if(*it < minn) minn = *it;
    return minn;
}

std::ostream& operator<<(std::ostream& os, Path p)
{
    os << "[";
    for(std::vector<double>::iterator it = p.path_.begin(); it != p.path_.end(); it++)
        os << *it << ",";
    return os<<"]";
}
std::ostream& operator<<(std::ostream& os, Path* p){return os<<(*p);}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  End
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

