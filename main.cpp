#include "tcx_standalone/t_blob.hpp"
#include "tcx_standalone/t_interval_map.hpp"
#include "tcx_standalone/t_random.hpp"
#include "tcx_standalone/t_shared.hpp"
#include "tcx_standalone/t_trie.hpp"
#include "tcx_standalone/t_file.hpp"
#include "tcx_standalone/t_math.hpp"
#include "tcx_standalone/t_visual.hpp"
#include <fstream>
#include <iostream>

int main(){
    
    tcx::PlaneFunction f1({{-1,2},{1,0}});
    tcx::PlaneFunction f2({{1,0.5}});
    std::vector<double> points;
    for(double i = -1 ;i<=1;i+=0.0001){
        points.emplace_back(i);
        points.emplace_back(f2(f1(i)));
    }
    std::vector<double> points2;
    bool flag = true;
    for(const auto&i:points){
        if(flag) points2.emplace_back(i);
        else points2.emplace_back(-i);
        flag = !flag;
    }

    tcx::Obj2D<double> obj2d = tcx::create_line(std::move(points));
    obj2d.color = {255,125,0,1};
    obj2d.set_scale(100,100);

    tcx::Obj2D<double> obj2d2 = tcx::create_line(std::move(points2));
    obj2d2.color = {255,0,0,1};
    obj2d2.set_scale(100,100);

    tcx::SVG svg(300,300);
    svg.add_obj(std::move(obj2d));
    svg.add_obj(std::move(obj2d2));
    
    std::ofstream ofs;
    std::string path = tcx::Path::cwd().cd("..").cd("test.svg").str();
    std::string content = svg.stringfy();
    tcx::overwrite(path,content.data(),content.size());

    return 0;
}