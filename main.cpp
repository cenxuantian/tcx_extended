#include "tcx_standalone/t_blob.hpp"
#include "tcx_standalone/t_interval_map.hpp"
#include "tcx_standalone/t_random.hpp"
#include "tcx_standalone/t_shared.hpp"
#include "tcx_standalone/t_trie.hpp"
#include "tcx_standalone/t_file.hpp"
#include "tcx_standalone/t_math.hpp"
#include "tcx_standalone/t_visual.hpp"
#include <iostream>

int main(){

    tcx::PlaneFunction f1({{1,2}});
    std::vector<tcx::Point2D<double>> points = f1.draw({0,10},0.1);
    tcx::SVG svg;
    tcx::Object2D<double> line = tcx::create_line<double>(std::move(*(std::vector<tcx::Any2D<double>>*)&points));
    line.color = {(unsigned char)255,0,0,1};
    line.scale = {100,100};
    svg.add(std::move(line));
    std::cout << svg.export_str();

    return 0;
}