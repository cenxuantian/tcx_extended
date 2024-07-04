#pragma once

#include <string.h>
#include <stdint.h>
#include <string>
#include <optional>

namespace tcx
{

struct Url{
    std::string protocal;
    std::string host;
    uint16_t port;
    std::string route;
    Url();
    Url(const char* str);
};

std::optional<Url> UrlParse(const char* str);
std::string Url2String(Url const&);






// ipml
namespace{

int find_first_of(char tar, const char* str,int sz, int start_pos = 0){
    for(int i = start_pos; i < sz;i++){
        if(str[i] == tar) return i-start_pos;
    }
    return -1;
}

int find_first_of(const char* tar, const char* str,int sz, int start_pos = 0){
    int len = strlen(tar);
    if(len == 0)return -1;
    for(int i = start_pos; i < sz;i++){
        if(str[i] == tar[0]){
            if(sz-i<len) return -1;
            int ret = memcmp(tar,str+i,len);
            if(ret == 0)return i-start_pos;
        }
    }
    return -1;
}

}

Url::Url(){

}
Url::Url(const char* str){
    std::optional<Url> urlopt = UrlParse(str);
    if(urlopt.has_value()){
        *this = std::move(urlopt.value());
    }
}

std::optional<Url> UrlParse(const char* str){
    int start_pos = 0;
    int sz = strlen(str);
    int pos;
    Url res;

    pos = find_first_of("://",str,sz,start_pos);
    if(pos<0){
        goto read_host;
    }
    res.protocal.assign(str+start_pos,pos);
    start_pos+=pos+3;

    read_host:
    pos = find_first_of(':',str,sz,start_pos);
    if(pos<0){
        pos = find_first_of('/',str,sz,start_pos);
        if(pos<0){
            res.host.assign(str+start_pos);
            res.port = 80;
            res.route = "/";
            return res;
        }else{
            res.host.assign(str+start_pos,pos);
            res.port = 80;
            res.route.assign(str+start_pos+pos);
            return res;
        }

    }else{
        res.host.assign(str+start_pos,pos);
        start_pos+=pos+1;
        pos = find_first_of('/',str,sz,start_pos);
        if(pos<0){
            res.port = atoi(str+start_pos);
            res.route = "/";
            return res;
        }
        else{
            std::string temp;
            temp.assign(str+start_pos,pos);
            res.port = atoi(temp.c_str());
            res.route.assign(str+start_pos+pos);
            return res;
        }
    }
    return {};
}

std::string Url2String(Url const& _url){
    return (_url.protocal.size()?_url.protocal+"://":"") + _url.host + (_url.port==80?"":std::to_string(_url.port)) + _url.route;

}


} // namespace tcx
