#pragma once
#include <future>
#include <optional>
#include <iostream>
#include "t_http_tools.hpp"
#include "t_sock_wrapper.hpp"
#include "../tcx_standalone/t_math.hpp"

namespace tcx{

enum class HTTPErrorType{
    CONNECT_TIMEOUT = 0,
    SERVER_NOT_RESPONSE = 1,
    SOCKET_IS_INVALID = 2,
    NO_CONNECTION = 3,
    SEND_ERROR = 4,
    RES_FORMAT_ERROR = 5,
};

class HTTPServer{
private:


public:

};

class HTTPClient{
private:
    Socket sock_;
    IPAddr addr_;
    bool connected_;
    std::function<void(HTTPErrorType)> err_cb_=[](HTTPErrorType){};
    template<HTTPErrorType _et>void __rise_error();
public:
    ~HTTPClient();
    HTTPClient();
    HTTPClient(IPAddr const&);
    void set_server_addr(IPAddr const&);
    std::optional<HTTPResponse> send(HTTPRequest const& req);
};


template<HTTPErrorType _et>void  HTTPClient::__rise_error(){
    err_cb_(_et);
}
HTTPClient::~HTTPClient(){
    sock_.close();
}
HTTPClient::HTTPClient():sock_(Socket::safe_create(Socket::Protocol::TCP)),connected_(false){

}
HTTPClient::HTTPClient(IPAddr const& addr)
    :sock_(Socket::safe_create(Socket::Protocol::TCP))
    ,connected_(false)
    ,addr_(addr)
{

}
void HTTPClient::set_server_addr(IPAddr const& addr){
    addr_ = addr;
}

std::optional<HTTPResponse> HTTPClient::send(HTTPRequest const& req){
    if(!sock_.valid()) {
        __rise_error<HTTPErrorType::SOCKET_IS_INVALID>();
        return {};
    }
    if(!connected_){
        if(sock_.connect(addr_)){
            goto send;
        }else{
            __rise_error<HTTPErrorType::CONNECT_TIMEOUT>();
            return {};
        }
    }
    send:
    if(sock_.write_all(HTTP_to_blob(req),std::chrono::milliseconds(3000))){

        // read
        std::optional<tcx::Blob> opt_blob = sock_.readuntill("\r\n\r\n");
        if(!opt_blob.has_value())return {};// read error
        std::optional<HTTPResponse> opt_res = HTTP_read_res(opt_blob.value());

        if(!opt_res.has_value())return {};
        HTTPResponse& res = opt_res.value();

        if(!sock_.readable()) return opt_res;// just head

        // read body
        if(res.headers.count("Content-Length")){
            int content_len = dec2i(res.headers["Content-Length"].c_str());// got content length
            if(content_len==-1 || content_len==0)return opt_res;// content len error or no content
            
            std::optional<tcx::Blob> res_opt2 = sock_.readall(content_len,std::chrono::milliseconds(1000));
            if(res_opt2.has_value()){
                res.body = std::move(res_opt2.value());
                return opt_res;
            }else return opt_res;// read error
        }else{
            read_body:
            std::optional<tcx::Blob> res_opt2 = sock_.readuntill("\r\n");
            if(!res_opt2.has_value()) return opt_res;// read error
            auto pos = res_opt2.value().find_first_of('\r');
            if(pos == std::string::npos)return opt_res;// content error
            int content_len = hex2i((char*)res_opt2.value().data(),(int)pos);
            if(content_len==-1 || content_len==0)return opt_res;// content len error or no content
            
            res_opt2 = sock_.readall(content_len,std::chrono::milliseconds(1000));
            if(res_opt2.has_value()){
                res.body << std::move(res_opt2.value());
                if(!sock_.readable()) return opt_res;// read finish
                else{
                    res_opt2 = sock_.readuntill("\r\n");
                    if(!res_opt2.has_value()) return opt_res;// read error
                    goto read_body;
                }
            }else return opt_res;// read error
            
        }

        return {};

    }else{
        int errorcode = sock_.fast_err();
        __rise_error<HTTPErrorType::SEND_ERROR>();
        return {};
    }
}

}