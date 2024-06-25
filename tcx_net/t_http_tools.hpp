#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <optional>
#include "../tcx_standalone/t_blob.hpp"

namespace tcx{

enum class HTTPReqType: int{
    T_OPTIONS = 0,
    T_HEAD = 1,
    T_GET = 2,
    T_POST = 3,
    T_PUT = 4,
    T_DELETE = 5,
    T_TRACE = 6,
    T_CONNECT = 7
};

enum class HTTPContentType :int{
    T_HTML = 0,
    T_XHTML = 1,
    T_PLAIN = 2,
    T_JSON = 3,
    T_CSS = 4,
    T_TEXT_XML = 5,
    T_APP_XML = 6,
    T_ATOM_XML = 7,
    T_GIF = 8,
    T_JPEG = 9,
    T_PNG = 10,
    T_X_JAVASCRIPT = 11,
    T_IMAGE_X_ICON = 12,
    T_PDF = 13,
    T_MSWORD = 14,
    T_OCTET_STREAM = 15,
    T_FORM = 16,
    T_DATA = 17,
};

struct HTTPRequest{
    HTTPReqType type = HTTPReqType::T_GET;
    std::string route;
    uint32_t version[2] = {1,1};
    std::unordered_map<std::string,std::string> headers;
    Blob body;
};

struct HTTPResponse{
    uint32_t version[2];
    int status_code;
    std::unordered_map<std::string,std::string> headers;
    Blob body;
};

namespace {

inline bool buf_equal(const void* buf,size_t sz, const char* str){
    size_t len = strlen(str);
    if(len!=sz)return false;
    else return memcmp(buf,str,sz)==0;
}

HTTPReqType get_type(const uint8_t* buf, size_t sz){
    if(buf_equal(buf,sz,"OPTIONS")){
        return HTTPReqType::T_OPTIONS;
    }else if(buf_equal(buf,sz,"HEAD")){
        return HTTPReqType::T_HEAD;
    }else if(buf_equal(buf,sz,"GET")){
        return HTTPReqType::T_GET;
    }else if(buf_equal(buf,sz,"POST")){
        return HTTPReqType::T_POST;
    }else if(buf_equal(buf,sz,"PUT")){
        return HTTPReqType::T_PUT;
    }else if(buf_equal(buf,sz,"DELETE")){
        return HTTPReqType::T_DELETE;
    }else if(buf_equal(buf,sz,"TRACE")){
        return HTTPReqType::T_TRACE;
    }else if(buf_equal(buf,sz,"CONNECT")){
        return HTTPReqType::T_CONNECT;
    }else{
        return HTTPReqType::T_GET;
    }
}
}


std::optional<HTTPRequest> HTTP_read_req(Blob & blob){
    HTTPRequest req;
    // remove useless ' '
    size_t pos = blob.find_first_not_of(' ');
    if(pos == std::string::npos) return {};
    blob.pop_front(pos);

    // get type
    pos = blob.find_first_of(' ');
    if(pos == std::string::npos) return {};
    req.type = get_type(blob.data(),pos);
    blob.pop_front(pos+1);

    // get route
    pos = blob.find_first_of(' ');
    if(pos == std::string::npos) return {};
    req.route.assign((char*)blob.data(),pos);
    blob.pop_front(pos+1);

    // get version
    pos = blob.find_first_of('/');
    if(pos == std::string::npos) return {};
    blob.pop_front(pos+1);
    pos = blob.find_first_of("\r\n");
    if(pos == std::string::npos) return {};
    std::string temp;
    temp.assign((char*)blob.data(),pos);
    double ver;
    try{ver = std::stod(temp);}
    catch(...) {return {};}
    req.version[0] =ver;
    req.version[1] =((double)ver-req.version[0])*10;


    return req;
}

HTTPResponse HTTP_read_res(Blob & blob){
    HTTPResponse res;
    return res;
}

Blob HTTP_to_blob(HTTPResponse const& res){
    return {};
}

Blob HTTP_to_blob(HTTPRequest const& res){
    return {};
}

}