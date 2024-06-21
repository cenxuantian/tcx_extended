#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <sstream>
#include <limits>
#include <math.h>

namespace tcx
{


// basic ----------------------------------------------------

template<typename T>
struct Any2D{
    T x;
    T y;
};

template<typename T>
struct Any3D{
    T x;
    T y;
    T z;
};

struct ColorRGBA{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    double a;
};

enum class ObjectType{
    T_0D = 0,
    T_1D = 1,
    T_2D = 2,
    T_3D = 3,
};

template<typename T>
struct Object2D{
    ObjectType type;
    Any2D<T> position;
    T rotation;
    Any2D<T> scale;
    std::vector<Any2D<T>> shape;
    ColorRGBA color;
    char* texture;
};


template<typename T>
Any2D<T> calculate_centroid(std::vector<Any2D<T>>const& points){
    Any2D<T> res = {0,0};
    for(auto & i : points){
        res.x +=i.x;
        res.y+= i.y;
    }
    res.x/=points.size();
    res.y/=points.size();
    return res;
}

template<typename T>
void rotate( Any2D<T>& rotate_point,Any2D<T> const& centroid, double radian){
    // radius
    T dx = centroid.x - rotate_point.x;
    T dy = centroid.y - rotate_point.y;
    T R = std::sqrt(std::pow(dx,2) + std::pow(dy,2));
    // angle 1
    double a1 = std::asin(dx/R);
    double a3 = radian + a1;
    dx = R * std::sin(a3);
    dy = R * std::cos(a3);
    rotate_point.x = centroid.x+dx;
    rotate_point.y = centroid.y+dy;
    return;
}

template<typename T>
Object2D<T> create_point(Any2D<T>&& point){
    return Object2D<T>{ObjectType::T_0D,{0,0},0,{1,1},{point},{1,1,1,1},0};
}

template<typename T>
Object2D<T> create_line(std::vector<Any2D<T>>&& points){
    return Object2D<T>{ObjectType::T_1D,{0,0},0,{1,1},points,{1,1,1,1},0};
}

template<typename T>
Object2D<T> create_polygon(std::vector<Any2D<T>>&& points){
    return Object2D<T>{ObjectType::T_2D,{0,0},0,{1,1},points,{1,1,1,1},0};
}


// svg ----------------------------------------------------------------


enum class SVGTag{
    T_RECT = 0,
    T_CIRCLE = 1,
    T_ELLIPSE = 2,
    T_LINE = 3,
    T_POLYLINE = 4,
    T_POLYGON = 5,
    T_PATH = 6
};

std::string to_string(SVGTag tag){
    switch (tag)
    {
    case SVGTag::T_RECT:return "rect";
    case SVGTag::T_CIRCLE:return "circle";
    case SVGTag::T_ELLIPSE:return "ellipse";
    case SVGTag::T_LINE:return "line";
    case SVGTag::T_POLYLINE:return "polyline";
    case SVGTag::T_POLYGON:return "polygon";
    case SVGTag::T_PATH:return "path";
    default:return "";
    }
    return "";
}

struct SVGComponent{
    SVGTag tag;
    bool need_end=false;
    std::vector<std::variant<char*,SVGComponent*>> content; // free for char* delete for SVCComponent*
    std::unordered_map<std::string,std::string> marks;
};


struct SVG{
    double version = 1.1;
    double width = 0;
    double height = 0;
    std::unordered_map<std::string,std::string> marks;
    std::vector<std::variant<char*,SVGComponent*>> content;

    void release(){
        for(std::variant<char*,SVGComponent*> & i : content){
            if(i.index() == 0){
                char* each = std::get<0>(i);
                if(each) free(each);
            }else{
                SVGComponent* each = std::get<1>(i);
                delete each;
            }
            
        }
        content = {};
    }

    void print_component(std::stringstream& ss,SVGComponent* each ) const {
        ss << '<'<<to_string(each->tag);
        for(const auto&j: each->marks){
            ss << ' '<<j.first<<"=\"" <<j.second<<"\"";
        }
        if(each->need_end){
            ss << '>';
            for(const auto& k : each->content){
                if(k.index()==0){
                    ss<< std::get<0>(k) << '\n';
                }else{
                    print_component(ss,std::get<1>(k));
                }
            }
            ss << "</"<<to_string(each->tag)<<">\n";
        }else{
            ss << "/>\n";
        }
    }
    

public:

    
    std::string export_str() const{
        std::stringstream ss;
        ss << "<svg width=\""<<std::floor(width)<<"\" height=\""<<std::floor(height)<<"\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"";
        for(const auto& i : marks){
            ss << ' '<<i.first<<"=\"" <<i.second<<"\"";
        }
        ss << ">\n";
        for(std::variant<char*,SVGComponent*> const& i : content){
            if(i.index()==0){
                ss<< std::get<0>(i) << '\n';
            }else{
                print_component(ss,std::get<1>(i));
            }
        }
        ss <<"</svg>";
        return ss.str();
    }

    void add(Object2D<double> && obj){
        SVGTag tag = SVGTag::T_POLYGON;
        switch (obj.type)
        {
        case ObjectType::T_0D:
        case ObjectType::T_1D:
            if(obj.shape.size() <=2) tag = SVGTag::T_LINE;
            else tag = SVGTag::T_POLYLINE;
            break;
        case ObjectType::T_2D:
        default:
            break;
        }
        SVGComponent* component = new SVGComponent{tag,false};
        component->content={};
        std::string color_str = "rgba("
            +std::to_string((int)std::round(obj.color.r))+","
            +std::to_string((int)std::round(obj.color.g))+","
            +std::to_string((int)std::round(obj.color.b))+","
            +std::to_string((int)std::round(obj.color.a))+")";


        // color
        if(tag ==  SVGTag::T_POLYGON){
            component->marks.emplace("fill",color_str);
        }else if(tag ==  SVGTag::T_POLYLINE || tag ==  SVGTag::T_LINE){
            component->marks.emplace("stroke",color_str);
            component->marks.emplace("fill","transparent");
            component->marks.emplace("stroke-width","5");
        }

        // shape
        if(tag ==  SVGTag::T_POLYGON || tag ==  SVGTag::T_POLYLINE){
            std::string points;
            Any2D<double> centroid = calculate_centroid(obj.shape);
            for(auto& i :obj.shape){
                // rotate(i,centroid,obj.rotation);
                i.x*=obj.scale.x;
                i.y*=obj.scale.y;
                i.x+=obj.position.x;
                i.y+= obj.position.y;
                if(i.x>width) width = i.x;
                if(i.y>height) height = i.y;
                points+= std::to_string((unsigned int)std::round(i.x)) + ' ' + std::to_string((unsigned int)std::round(i.y)) + ", ";
            }
            points.resize(points.size()-2);
            component->marks.emplace("points",points);
        }
        else if (tag == SVGTag::T_LINE){
            Any2D<double> centroid = calculate_centroid(obj.shape);
            size_t cur_name = 1;
            for(auto& i :obj.shape){
                // rotate(i,centroid,obj.rotation);
                i.x*=obj.scale.x;
                i.y*=obj.scale.y;
                i.x+=obj.position.x;
                i.y+= obj.position.y;
                if(i.x>width) width = i.x;
                if(i.y>height) height = i.y;
                component->marks.emplace("x"+std::to_string(cur_name),std::to_string((unsigned int)std::round(i.x)));
                component->marks.emplace("y"+std::to_string(cur_name),std::to_string((unsigned int)std::round(i.y)));
                cur_name++;
            }
            
        }

        component->need_end = false;
        std::variant<char*,SVGComponent*> each;
        each.emplace<1>(component);
        content.emplace_back(std::move(each));
        return;
    }


};

} // namespace tcx
