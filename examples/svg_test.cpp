#include <t_file.hpp>
#include <t_math.hpp>
#include <t_visual.hpp>
#include <fstream>
#include <iostream>

void create_circle(std::vector<double>& points_circle1,std::vector<double>& points_circle2){
    tcx::PlaneFunction f1({{-1,2},{1,0}});
    tcx::PlaneFunction f2({{1,0.5}});
    
    for(double i = -1 ;i<=1;i+=0.0001){
        points_circle1.emplace_back(i);
        points_circle1.emplace_back(f2(f1(i)));
    }
    
    bool flag = true;
    for(const auto&i:points_circle1){
        if(flag) points_circle2.emplace_back(i);
        else points_circle2.emplace_back(-i);
        flag = !flag;
    }
}

int main(){
    std::vector<double> points_circle1;
    std::vector<double> points_circle2;
    create_circle(points_circle1,points_circle2);
    
    tcx::Obj2D<double> circle1 = tcx::create_line(std::move(points_circle1));
    circle1.color = {255,125,0,1};
    circle1.set_scale(100,100);

    tcx::Obj2D<double> circle2 = tcx::create_line(std::move(points_circle2));
    circle2.color = {255,0,0,1};
    circle2.set_scale(100,100);

    tcx::SVG svg(300,300);
    svg.add_obj(std::move(circle1));
    svg.add_obj(std::move(circle2));
    svg.add_axis();
    
    std::ofstream ofs;
    std::string path = tcx::Path::cwd().cd("..").cd("test.svg").str();
    std::string content = svg.stringfy();
    tcx::overwrite(path,content.data(),content.size());

    return 0;
}