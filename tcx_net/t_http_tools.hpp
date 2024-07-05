#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <optional>
#include "../tcx_standalone/t_blob.hpp"
#include "t_sock_wrapper.hpp"

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
    std::string route = "/";
    uint32_t version[2] = {1,1};
    std::unordered_map<std::string,std::string> headers = {};
    Blob body;
};

struct HTTPResponse{
    uint32_t version[2]= {1,1};
    int status_code =404;
    std::unordered_map<std::string,std::string> headers={};
    Blob body;
};

namespace {

inline bool buf_equal(const void* buf,size_t sz, const char* str){
    size_t len = strlen(str);
    if(len!=sz)return false;
    else return memcmp(buf,str,sz)==0;
}

inline HTTPReqType get_type(const uint8_t* buf, size_t sz){
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

inline std::string to_string(HTTPReqType reqtype){
    switch (reqtype)
    {
    case HTTPReqType::T_OPTIONS:return "OPTIONS";
    case HTTPReqType::T_HEAD:return "HEAD";
    case HTTPReqType::T_GET:return "GET";
    case HTTPReqType::T_POST:return "POST";
    case HTTPReqType::T_PUT:return "PUT";
    case HTTPReqType::T_DELETE:return "DELETE";
    case HTTPReqType::T_TRACE:return "TRACE";
    case HTTPReqType::T_CONNECT:return "CONNECT";
    default: return "GET";
}

}

inline std::string get_status_str(int status_code){
    switch (status_code){
        case 100: return "Continue";
        case 101: return "Switching Protocol";
        case 200: return "OK";
        case 201: return "Created";
        case 202: return "Accepted";
        case 203: return "Non-Authoritative Information";
        case 204: return "No Content";
        case 205: return "Reset Content";
        case 206: return "Partial Content";
        case 300: return "Multiple Choices";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        case 305: return "Use Proxy";
        case 306: return "Unused";
        case 307: return "Temporary Redirect";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 402: return "Payment Required";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 407: return "Proxy Authentication Required";
        case 408: return "Request Time-out";
        case 409: return "Conflict";
        case 410: return "Gone";
        case 411: return "Length Required";
        case 412: return "Precondition Failed";
        case 413: return "Request Entity Too Large";
        case 414: return "Request-URI Too Large";
        case 415: return "Unsupported Media Type";
        case 416: return "Requested range not satisfiable";
        case 417: return "Expectation Failed";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Time-out";
        case 505: return "HTTP Version not supported";
        default:  return "Status Error";
    }
}

}

std::optional<HTTPRequest> HTTP_read_req(Blob const& blob){
    HTTPRequest req;
    size_t offset = 0;

    // remove useless ' '
    size_t pos = blob.find_first_not_of(' ',offset);
    if(pos == std::string::npos) return {};
    offset+=pos;

    // get type
    pos = blob.find_first_of(' ',offset);
    if(pos == std::string::npos) return {};
    req.type = get_type(blob.data(offset),pos);
    offset+=pos+1;

    // get route
    pos = blob.find_first_of(' ',offset);
    if(pos == std::string::npos) return {};
    req.route.assign((char*)blob.data(offset),pos);
    offset+=pos+1;

    // get version
    pos = blob.find_first_of('/',offset);
    if(pos == std::string::npos) return {};
    offset+=pos+1;
    pos = blob.find_first_of("\r\n",offset);
    if(pos == std::string::npos) return {};
    std::string temp;
    temp.assign((char*)blob.data(offset),pos);
    double ver;
    try{ver = std::stod(temp);}
    catch(...) {return {};}
    req.version[0] =ver;
    req.version[1] =((double)ver-req.version[0])*10;
    // blob.pop_front(pos+2);
    offset+=pos+2;

    // get headers
    std::string temp_key;
    std::string temp_val;
    get_header:
    {
        // gete key
        pos = blob.find_first_not_of(' ',offset);
        if(pos == std::string::npos) return req;
        offset+=pos;  // remove useless space
        pos = blob.find_first_of(':',offset);
        if(pos == std::string::npos) return req;
        temp_key.clear();
        temp_key.assign((char*)blob.data(offset),pos);
        offset+=pos+1;
    }
    {
        // get val
        pos = blob.find_first_not_of(' ',offset);
        if(pos == std::string::npos) return req;
        offset+=pos;  // remove useless space
        pos = blob.find_first_of("\r\n",offset);
        if(pos == std::string::npos) return req;
        temp_val.clear();
        temp_val.assign((char*)blob.data(offset),pos);
        offset+=pos;// keep the \r
    }

    req.headers.emplace(std::move(temp_key),std::move(temp_val));
    if(blob.size()>=4){
        if(!buf_equal(blob.data(offset),4,"\r\n\r\n")){
            offset+=2;
            goto get_header;
        }else{
            offset+=4;
            req.body.assign(blob.data(offset),blob.size()-offset,0);
            return req;
        }
    }else{
        return req;
    }
}

std::optional<HTTPResponse> HTTP_read_res(Blob const& blob){
    HTTPResponse res;
    size_t offset = 0;

    // remove useless ' '
    size_t pos = blob.find_first_not_of(' ',offset);
    if(pos == std::string::npos) return {};
    offset+=pos;
    // blob.pop_front(pos);

    // get version
    pos = blob.find_first_of('/',offset);
    if(pos == std::string::npos) return {};
    offset+=pos+1;
    // blob.pop_front(pos+1);
    pos = blob.find_first_of(' ',offset);
    if(pos == std::string::npos) return {};
    std::string temp;
    temp.assign((char*)blob.data(offset),pos);
    double ver;
    try{ver = std::stod(temp);}
    catch(...) {return {};}
    res.version[0] =ver;
    res.version[1] =((double)ver-res.version[0])*10;
    // blob.pop_front(pos+1);
    offset+=pos+1;


    // get code 
    pos = blob.find_first_of(' ',offset);
    if(pos == std::string::npos) return {};
    std::string code_str;
    code_str.assign((char*)blob.data(offset),pos);
    try{res.status_code = std::stoi(code_str);}catch(...){return {};}
    // blob.pop_front(pos+1);
    offset+=pos+1;

    // get headers prepare
    pos = blob.find_first_of("\r\n",offset);
    if(pos == std::string::npos) return {};
    // blob.pop_front(pos+2);
    offset+=pos+2;

    // get headers
    std::string temp_key;
    std::string temp_val;
    get_header:
    {
        // gete key
        pos = blob.find_first_not_of(' ',offset);
        if(pos == std::string::npos) return res;
        offset+=pos; // remove useless space
        // blob.pop_front(pos);  // remove useless space
        pos = blob.find_first_of(':',offset);
        if(pos == std::string::npos) return res;
        temp_key.clear();
        temp_key.assign((char*)blob.data(offset),pos);
        // blob.pop_front(pos+1);
        offset+=pos+1;
    }
    {
        // get val
        pos = blob.find_first_not_of(' ',offset);
        if(pos == std::string::npos) return res;
        offset+=pos; // remove useless space
        // blob.pop_front(pos);  // remove useless space
        pos = blob.find_first_of("\r\n",offset);
        if(pos == std::string::npos) return res;
        temp_val.clear();
        temp_val.assign((char*)blob.data(offset),pos);
        // blob.pop_front(pos);// keep the \r
        offset+=pos;
    }

    res.headers.emplace(std::move(temp_key),std::move(temp_val));
    if(blob.size()>=4){
        if(!buf_equal(blob.data(offset),4,"\r\n\r\n")){
            // blob.pop_front(2);
            offset+=2;
            goto get_header;
        }else{
            // blob.pop_front(4);
            offset+=4;
            res.body.assign(blob.data(offset),blob.size()-offset,0);
            // res.body = std::move(blob);
            return res;
        }
    }else{
        return res;
    }
    return res;
}

Blob HTTP_to_blob(HTTPRequest const& req){
    Blob bin;
    bin << to_string(req.type) << ' ' << req.route
        << " HTTP/"<< std::to_string(req.version[0]) <<'.'<< std::to_string(req.version[1]) <<"\r\n";
    for(const auto& i : req.headers){
        bin << i.first << ": "<<i.second <<"\r\n";
    }
    bin << "\r\n"<<req.body<<0;
    return bin;
}

Blob HTTP_to_blob(HTTPResponse const& res){
    Blob bin;
    bin << "HTTP/"<< std::to_string(res.version[0]) <<'.'<< std::to_string(res.version[1]) << ' ' 
        << std::to_string(res.status_code) <<' '<<get_status_str(res.status_code) <<"\r\n";
    for(const auto& i : res.headers){
        bin << i.first << ": "<<i.second<<"\r\n";
    }
    bin << "\r\n"<<res.body<<0;
    return bin;
}




}