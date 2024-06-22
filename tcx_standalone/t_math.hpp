#pragma once
#include <vector>
#include <math.h>
#include <memory>
#include <utility>

namespace tcx{

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

    double operator()(double x) const noexcept{ return solution(x);}
    
    PlaneFunction derivative()const noexcept{
        PlaneFunction res = *this;
        for(auto & i : res.desc){
            i.coefficient = i.exponent;
            i.exponent--;
        }
        return res;
    }

    std::vector<double> draw(double start, double stop, double interval) const noexcept{
        std::vector<double> points;
        for(double i = start; i<stop;i+=interval){
            points.emplace_back(i);
            points.emplace_back(solution(i));
        }
        return points;
    } 

};


}