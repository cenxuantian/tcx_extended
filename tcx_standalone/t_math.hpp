#pragma once
#include <vector>
#include <math.h>
#include <memory>
#include <utility>
#include <unordered_map>
#include <string.h>

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

inline int hex2i(char hex_char){
    if(hex_char >='0' && hex_char<='9')return hex_char-'0';
    else if(hex_char >='a' && hex_char <='f') return hex_char-'a'+10;
    else if(hex_char >='A' && hex_char <='F') return hex_char-'A'+10;
    else return -1;
}

inline int hex2i(const char* hex_str,int size = -1){
    int sz = size==-1?strlen(hex_str):size;
    int sum = 0;
    for(int i =0;i<sz;i++){
        int cur =hex2i(hex_str[i]);
        if(cur==-1)return -1;
        sum+= cur * std::pow<int>(16,sz-1-i);
    }
    return sum;
}

inline int dec2i(char dec_char){
    if(dec_char >='0' && dec_char<='9')return dec_char-'0';
    else return -1;
}

inline int dec2i(const char* dec_str,int size = -1){
    int sz = size==-1?strlen(dec_str):size;
    int sum = 0;
    for(int i =0;i<sz;i++){
        int cur =dec2i(dec_str[i]);
        if(cur==-1)return -1;
        sum+= cur * std::pow<int>(10,sz-1-i);
    }
    return sum;
}

}