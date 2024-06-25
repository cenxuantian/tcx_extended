#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <sstream>
#include <limits>
#include <math.h>
#include <utility>

namespace tcx
{
// basic ----------------------------------------------------

template<typename T>
struct Point2D{
    T x;
    T y;
};

template<typename T>
struct Point3D{
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
struct Obj2D{
    ObjectType type;
    T position[2];
    T scale[2];
    T rotation;
    ColorRGBA color;
    std::unique_ptr<char> texture;
    std::vector<T> shape;

    void move_by(T x, T y){
        position[0] += x;
        position[1] += y;
    }
    void move_to(T x, T y){
        position[0] = x;
        position[1] = y;
    }
    void set_scale(T x, T y){
        scale[0]=x;
        scale[1]=y;
    }
};

template<typename T>
Point2D<T> calculate_centroid_2d(std::vector<T> const& points){
    Point2D<T> res = {0,0};
    bool flag = true;
    for(const auto& i :points){
        if(flag){
            res.x+=i;
        }else{
            res.y+=i;
        }
        flag = !flag;
    }
    res.x/=points.size()/2;
    res.y/=points.size()/2;
    return res;
}

// template<typename T>
// void rotate( Any2D<T>& rotate_point,Any2D<T> const& centroid, double radian){
//     // radius
//     T dx = centroid.x - rotate_point.x;
//     T dy = centroid.y - rotate_point.y;
//     T R = std::sqrt(std::pow(dx,2) + std::pow(dy,2));
//     // angle 1
//     double a1 = std::asin(dx/R);
//     double a3 = radian + a1;
//     dx = R * std::sin(a3);
//     dy = R * std::cos(a3);
//     rotate_point.x = centroid.x+dx;
//     rotate_point.y = centroid.y+dy;
//     return;
// }

template<typename T>
Obj2D<T> create_line(std::vector<T>&& points){
    return {
        ObjectType::T_1D,
        {0,0},
        {1,1},
        0,
        {1,1,1,1},
        {nullptr},
        std::move(points)
    };
}

template<typename T>
Obj2D<T> create_polygon(std::vector<T>&& points){
    return {
        ObjectType::T_2D,
        {0,0},
        {1,1},
        0,
        {1,1,1,1},
        {nullptr},
        std::move(points)
    };
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
    SVGTag tag = SVGTag::T_LINE;
    bool need_end=false;
    std::vector<std::variant<char*,SVGComponent*>> content{}; // free for char* delete for SVCComponent*
    std::unordered_map<std::string,std::string> marks{};
};


class SVG{
private:
    double version = 1.1;
    size_t width = 0;
    size_t height = 0;
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
    
    SVGTag get_svg_tag(Obj2D<double> const& obj){
        SVGTag tag = SVGTag::T_POLYGON;
        switch (obj.type)
        {
        case ObjectType::T_0D:
        case ObjectType::T_1D:
            if(obj.shape.size() <=4) tag = SVGTag::T_LINE;
            else tag = SVGTag::T_POLYLINE;
            break;
        case ObjectType::T_2D:
        default:
            break;
        }
        return tag;
    }

public:
    SVG(size_t _height, size_t _width):height(_height),width(_width){}
    ~SVG(){ release();}

    std::string stringfy() const{
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

    void add_obj(Obj2D<double> && obj){
        SVGComponent* component = new SVGComponent;
        component->tag = get_svg_tag(obj);
        component->need_end = false;
        component->marks={};
        component->content={};
        std::string color_str = "rgba("
            +std::to_string((int)std::round(obj.color.r))+","
            +std::to_string((int)std::round(obj.color.g))+","
            +std::to_string((int)std::round(obj.color.b))+","
            +std::to_string((int)std::round(obj.color.a))+")";

        // color
        if(component->tag ==  SVGTag::T_POLYGON){
            component->marks.emplace("fill",color_str);
        }else if(component->tag ==  SVGTag::T_POLYLINE || component->tag ==  SVGTag::T_LINE){
            component->marks.emplace("stroke",color_str);
            component->marks.emplace("fill","transparent");
            component->marks.emplace("stroke-width","2");
        }

        // shape
        std::string points;
        Point2D<double> centroid = calculate_centroid_2d(obj.shape);
        double* points_arr = obj.shape.data();
        size_t points_count = obj.shape.size();
        size_t cur_name = 1;
        double center_x = width/2;
        double center_y = height/2;
        for(size_t i = 0 ;i< points_count;i+=2){
            // rotate(i,centroid,obj.rotation);
            points_arr[i]   *=  obj.scale[0];
            points_arr[i+1] *=  obj.scale[1];
            points_arr[i]   +=  obj.position[0];
            points_arr[i+1] +=  obj.position[1];

            points_arr[i] += center_x;
            points_arr[i+1] = center_y - points_arr[i+1];

            if(component->tag == SVGTag::T_POLYGON || component->tag == SVGTag::T_POLYLINE){
                points+= std::to_string((int)std::round(points_arr[i])) + ' ' + std::to_string((int)std::round(points_arr[i+1])) + ", ";
            }

            else if (component->tag == SVGTag::T_LINE){
                component->marks.emplace("x"+std::to_string(cur_name),std::to_string((unsigned int)std::round(points_arr[i])));
                component->marks.emplace("y"+std::to_string(cur_name),std::to_string((unsigned int)std::round(points_arr[i+1])));
                cur_name++;
            }
        }

        points.resize(points.size()-2);
        component->marks.emplace("points",points);

        std::variant<char*,SVGComponent*> each;
        each.emplace<1>(component);
        content.emplace_back(std::move(each));
        return;
    }

    void add_axis(){
        {
            SVGComponent* component = new SVGComponent;
            component->tag = SVGTag::T_LINE;
            component->need_end = false;
            component->marks={};
            component->content={};
            component->marks.emplace("stroke","rgba(0,0,255,1)");
            component->marks.emplace("fill","transparent");
            component->marks.emplace("stroke-width","1");
            component->marks.emplace("x1","0");
            component->marks.emplace("y1",std::to_string(height/2));
            component->marks.emplace("x2",std::to_string(width));
            component->marks.emplace("y2",std::to_string(height/2));
            std::variant<char*,SVGComponent*> each;
            each.emplace<1>(component);
            content.emplace_back(std::move(each));
        }
        {
            SVGComponent* component = new SVGComponent;
            component->tag = SVGTag::T_LINE;
            component->need_end = false;
            component->marks={};
            component->content={};
            component->marks.emplace("stroke","rgba(0,255,0,1)");
            component->marks.emplace("fill","transparent");
            component->marks.emplace("stroke-width","1");
            component->marks.emplace("x1",std::to_string(width/2));
            component->marks.emplace("y1","0");
            component->marks.emplace("x2",std::to_string(width/2));
            component->marks.emplace("y2",std::to_string(height));
            std::variant<char*,SVGComponent*> each;
            each.emplace<1>(component);
            content.emplace_back(std::move(each));
        }
    }

};

} // namespace tcx
