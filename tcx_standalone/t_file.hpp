#pragma once

#include <stdint.h>
#include <fstream>
#include <string>
#include <stack>
#include <algorithm>
#include <optional>
#include <vector>

namespace tcx
{

class Path{

private:
    
    std::stack<std::string> stack_;

    inline static std::string get_cwd(){
#ifdef _WIN32
        DWORD actual = GetCurrentDirectoryA(0,nullptr);
        if(!actual) return "";
        char* a = (char*)malloc(actual);
        if(!a) return "";
        GetCurrentDirectoryA(actual,a);
        std::string res = a;
        free(a);
        return res;
#elif defined(__linux__)
#ifdef __GNUC__
        char* a = ::getcwd(nullptr,0);
        std::string res = a;
        free(a);
        return res;
#else
        return "";
#endif
#else
        return "";
#endif
    }

    inline static bool is_delimiter(char i)noexcept{
#ifdef _WIN32
    return (i=='/' || i=='\\');
#elif defined(__linux__)
    return i=='/';
#elif defined(__APPLE__)
    return i=='/';
#endif
    return false;
    }

    inline static char get_delimiter()noexcept{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
    }

    inline static size_t find_first_delimiter(std::string const& _path_str)noexcept{
#ifdef _WIN32
    return std::min(_path_str.find_first_of('/'),_path_str.find_first_of('\\'));
#else
    return _path_str.find_first_of('/');
#endif
    }
public:
    Path():stack_(){}

    static Path parse(std::string const& _path_str){
        size_t start_pos = 0;
        size_t size = _path_str.size();
        Path res;
        if(!size)return res;


        size_t pos = find_first_delimiter(_path_str);
        std::string first_path = _path_str.substr(0,pos);
        
        if(!first_path.size()){
            res.stack_.emplace("");
        }else if(first_path == "." || first_path == ".."){
            res.stack_.emplace(first_path);
        }
#ifdef _WIN32
        else if(first_path.find_first_of(':')!=std::string::npos){
            res.stack_.emplace(first_path);
        }
#endif
        else {
            res.stack_.emplace(".");
            res.stack_.emplace(first_path);
        }
        if(pos == std::string::npos)return res;
        start_pos=pos+1;
#ifdef _WIN32
#endif
        std::string temp="";
        for(size_t i= start_pos;i<size;i++){
            if(is_delimiter(_path_str[i])){
                if(temp.size() && temp !="." && temp!=".."){
                    res.stack_.emplace(temp);
                }
                else if(temp == "..") {
                    if (res.stack_.top() == "."){
                        res.stack_.top() = "..";    // 
                    }else if(res.stack_.top() == ".."){
                        res.stack_.emplace("..");
                    }
#ifdef _WIN32
                    else if(res.stack_.top().find_first_of(':')!=std::string::npos){}// cannot cd ..
#else
                    else if (res.stack_.top() == ""){}// cannot cd ..
#endif
                    else{
                        res.stack_.pop();
                    }
                }
                else if(temp == "."){}
                temp="";
            }
            else{
                temp+=_path_str[i];
            }
        }
        if(temp.size()) res.stack_.emplace(std::move(temp));
        return res;
    }   

    static Path cwd(){ return parse(get_cwd());}

    std::string str() const{
        std::stack<std::string> copy_stack = stack_;
        if(!stack_.size()){
#ifdef _WIN32
            return "";
#else
            return "/";
#endif
        }
        std::string res="";
        while(!copy_stack.empty()){
            if(copy_stack.size() != 1){
                res = get_delimiter() + copy_stack.top() + res;
            }else{
                res = copy_stack.top() + res;
            }
            copy_stack.pop();
        }
        return res;
    }

    Path& join(Path&& other_path){
        std::vector<std::string> vec;
        vec.reserve(other_path.stack_.size());
        while(other_path.stack_.size()){
            vec.emplace_back(std::move(other_path.stack_.top()));
            other_path.stack_.pop();
        }
        if(vec.size()){
            if(vec[0]==""){
                
#ifdef _WIN32
                if(stack_.size()){
                    while(stack_.size()>1){
                        stack_.pop();
                    }
                    vec[0]= stack_.top();
                    stack_.pop();
                }
#else
                stack_={};
#endif
                for(auto&i:vec){
                    stack_.push(std::move(i));
                }
                return *this;
            }
        }
        for(auto i = vec.rbegin(), end = vec.rend(); i!=end;++i){
            if(*i == "."){}
            else if(*i==".."){
                if (stack_.top() == "."){
                    stack_.top() = "..";    // 
                }else if(stack_.top() == ".."){
                    stack_.emplace("..");
                }
#ifdef _WIN32
                else if(stack_.top().find_first_of(':')!=std::string::npos){}// cannot cd ..
#else
                else if (stack_.top() == ""){}// cannot cd ..
#endif
                else{
                    stack_.pop();
                }
            }else if(i->size()){
                stack_.push(std::move(*i));
            }
        }
        return *this;
    }

    Path& cd(std::string const& tar){return join(parse(tar));}

    bool valid()const{

    }
};

} // namespace tcx
