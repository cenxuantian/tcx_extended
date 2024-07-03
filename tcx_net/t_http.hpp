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
    if(sock_.write_all(HTTP_to_blob(req),std::chrono::milliseconds(1000))){
        // send ok, wait for respons
        std::optional<tcx::Blob> res_opt = sock_.readall(2048,std::chrono::milliseconds(1000));
        if(!res_opt.has_value()){
            __rise_error<HTTPErrorType::SERVER_NOT_RESPONSE>();
            return {};
        }
        auto& b = res_opt.value();
        std::cout << b.data() << "\n\n\n";
        std::optional<HTTPResponse> res = HTTP_read_res(res_opt.value());
        if(res.has_value()){
            auto& value = res.value();
            if(value.headers.count("Content-Length")){
                int content_len = 0;    // got content length
                try{
                    content_len = std::stoi(value.headers["Content-Length"]);
                }catch(...){
                    // content len error
                    return res;
                }
                if(content_len>value.body.size()){
                    int left_size = content_len - value.body.size();
                    std::optional<tcx::Blob> res_opt2 = sock_.readall(left_size,std::chrono::milliseconds(1000));
                    if(res_opt2.has_value()){
                        res.value().body << res_opt2.value();
                        return res;
                    }else return res;
                }
            }
            else{
                // no content length
                auto& body = res.value().body;
                size_t pos = body.find_first_of("\r\n");
                if(pos>=64)return res;// no lens indecator

                std::string s_size;
                s_size.assign((char*)body.data(),pos);
                int content_len = hex2dec(s_size);
                body.pop_back(pos+2);
                int left_size = content_len-body.size();
                std::optional<tcx::Blob> res_opt2 = sock_.readall(left_size,std::chrono::milliseconds(1000));
                if(!res_opt2.has_value()) return res;
                res.value().body << res_opt2.value();


                std::optional<tcx::Blob> res_opt3 = sock_.readsome(2);
                if(!res_opt3.has_value())return res; // no more
                if(!(res_opt3.value()[0] == '\r' && res_opt3.value()[1] == '\n')) return res;
                
                return res;

            }
            return res;
        }else{
            __rise_error<HTTPErrorType::RES_FORMAT_ERROR>();
            return {};
        }
    }else{
        int errorcode = sock_.fast_err();
        __rise_error<HTTPErrorType::SEND_ERROR>();
        return {};
    }
}

}