#pragma once

#include <stdint.h>
#include  <io.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <stack>
#include <algorithm>
#include <optional>
#include <vector>
#include <memory>
#include <windows.h>

namespace tcx
{

class Path{

private:
    
    std::stack<std::string> stack_;
    std::string str_cache_;
    bool changed;

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
    Path():stack_(),str_cache_(""),changed(false){}

    static Path parse(std::string const& _path_str){
        size_t start_pos = 0;
        size_t size = _path_str.size();
        Path res;
        if(!size)return res;
        res.changed=true;

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

    std::string const& str() const{
        if(!changed) return str_cache_;
        const_cast<bool&>(changed) = false;
        std::stack<std::string> copy_stack = stack_;
        if(!stack_.size()){
#ifdef _WIN32
            const_cast<std::string&>(str_cache_) = "";
#else
            const_cast<std::string&>(str_cache_) = "/";
#endif
            return str_cache_;
        }
        const_cast<std::string&>(str_cache_)="";
        while(!copy_stack.empty()){
            if(copy_stack.size() != 1){
                const_cast<std::string&>(str_cache_) = get_delimiter() + copy_stack.top() + str_cache_;
            }else{
                const_cast<std::string&>(str_cache_) = copy_stack.top() + str_cache_;
            }
            copy_stack.pop();
        }
        return str_cache_;
    }

    Path& join(Path&& other_path){
        if(!other_path.stack_.size()) return *this;
        changed=true;
        size_t sz = other_path.stack_.size();
        std::vector<std::string> vec(sz);
        for(size_t i=0;i< sz;i++){
            vec[sz-i-1] = std::move(other_path.stack_.top());
            other_path.stack_.pop();
        }

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

    bool valid()const{ int ret = _access(str().c_str(),0); return ret !=ENOENT && ret!=-1;}

    std::vector<_finddata_t> ll()const{
        std::vector<_finddata_t> res{};
        struct _finddata_t fileinfo{};
        std::string to_search_s = str();
        to_search_s+=get_delimiter();
        to_search_s+='*';
        auto handle=_findfirst(to_search_s.c_str(),&fileinfo);
        if(-1==handle) return res;
        do if(strcmp(fileinfo.name,".") != 0 && strcmp(fileinfo.name,"..") != 0) res.emplace_back(fileinfo);
        while(!_findnext(handle,&fileinfo));
        _findclose(handle);
        return res;
    }

    std::vector<std::string> ls()const{
        std::vector<std::string> res{};
        struct _finddata_t fileinfo{};
        std::string to_search_s = str();
        to_search_s+=get_delimiter();
        to_search_s+='*';
        auto handle=_findfirst(to_search_s.c_str(),&fileinfo);
        if(-1==handle) return res;
        do if(strcmp(fileinfo.name,".") != 0 && strcmp(fileinfo.name,"..") != 0) res.emplace_back(fileinfo.name);
        while(!_findnext(handle,&fileinfo));
        _findclose(handle);
        return res;
    }

    inline std::vector<_finddata_t> dir()const{return ll();}

    bool mkdir(std::string const& tar) const{
        Path temp = *this;
        temp.join(parse(tar));
        return ::mkdir(temp.str().c_str()) == 0;
    }


};

inline void fwrite(std::ofstream & ofs, char* buf, size_t sz){
    ofs.write(buf,sz);
}

inline std::pair<std::unique_ptr<char>,size_t> fread(std::ifstream & ifs){
    size_t org_pos = ifs.tellg();
    ifs.seekg(org_pos,std::ios::end);
    size_t last_pos = ifs.tellg();
    size_t sz = last_pos-org_pos;
    ifs.seekg(org_pos);

    std::unique_ptr<char> res(new char[sz]);
    ifs.read(res.get(),sz);
    return {std::move(res),sz};
}


template<std::ios_base::openmode openmode = std::ios::binary>
inline std::pair<std::unique_ptr<char>,size_t> read(std::string const& path){
    std::ifstream ifs;
    ifs.open(path,std::ios::in|openmode);
    if(!ifs.is_open()) return {std::unique_ptr<char>(nullptr),0};
    auto res(tcx::fread(ifs));
    ifs.close();
    return std::move(res);
}

template<std::ios_base::openmode openmode = std::ios::binary>
inline bool overwrite(std::string const& path, char* buf, size_t sz){
    std::ofstream ofs;
    ofs.open(path,std::ios::trunc|openmode);
    if(!ofs.is_open()) return false;
    tcx::fwrite(ofs,buf,sz);
    ofs.close();
    return true;
};

template<std::ios_base::openmode openmode = std::ios::binary>
inline bool append(std::string const& path, char* buf, size_t sz){
    std::ofstream ofs;
    ofs.open(path,std::ios::app|openmode);
    if(!ofs.is_open()) return false;
    tcx::fwrite(ofs,buf,sz);
    ofs.close();
    return true;
};

template<std::ios_base::openmode openmode = std::ios::binary>
inline bool create(std::string const& path, char* buf, size_t sz){
    return tcx::append(path,buf,sz);
};


} // namespace tcx
