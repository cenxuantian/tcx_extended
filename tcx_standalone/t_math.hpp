#pragma once
#include <vector>
#include <math.h>

namespace tcx{

template<typename T = double>
struct Point2D{
    T x;
    T y;
};

template<typename T = double>
struct Point3D{
    T x;
    T y;
    T z;
};

template<typename T = double>
struct Range{
    T start;
    T stop;
};



struct IndependentVarDesc{
    double coefficient;
    double exponent;
};

class PlaneFunction{
private:
    std::vector<IndependentVarDesc> desc;
public:
    PlaneFunction(std::vector<IndependentVarDesc> const& _desc):desc(_desc){}
    
    double solution(double x) const noexcept{
        double y = 0;
        for(const auto&i:desc){
            y+= i.coefficient * std::pow(x,i.exponent);
        }
        return y;
    }
    
    PlaneFunction derivative()const noexcept{
        PlaneFunction res = *this;
        for(auto & i : res.desc){
            i.coefficient = i.exponent;
            i.exponent--;
        }
        return res;
    }
    
    std::vector<Point2D<double>> draw(Range<double> const& range, double interval)const noexcept{
        std::vector<Point2D<double>> res;
        for(double i = range.start; i<range.stop;i+=interval){
            res.emplace_back(Point2D<double>{i,solution(i)});
        }
        return res;
    }

};


}