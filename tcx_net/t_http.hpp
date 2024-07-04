#pragma once

#include "t_http_tools.hpp"
#include "t_sock_wrapper.hpp"
#include "../tcx_standalone/t_math.hpp"

#include <future>
#include <optional>
#include <iostream>
#include <regex>
#include <utility>
#include <unordered_set>


namespace tcx{
// pre def
class HTTPServer;
class HTTPClient;
enum class HTTPErrorType;
class WebSocketServer;
class WebSocketClient;
std::pair<HTTPClient,std::optional<HTTPResponse>> HTTP_GET(Url const& url,std::unordered_map<std::string,std::string> && addtional_headers = {});
std::pair<HTTPClient,std::optional<HTTPResponse>> HTTP_POST(Url const& url, Blob && msg,std::unordered_map<std::string,std::string> && addtional_headers = {});
std::optional<HTTPResponse> HTTP_read_res_from_sock(Socket & sock_);
std::optional<HTTPRequest> HTTP_read_req_from_sock(Socket & sock_);

// interface

enum class HTTPErrorType{
    CONNECT_TIMEOUT = 0,
    SERVER_NOT_RESPONSE = 1,
    SOCKET_IS_INVALID = 2,
    NO_CONNECTION = 3,
    SEND_ERROR = 4,
    RES_FORMAT_ERROR = 5,
    ADDR_USED = 6,
    UNKNOWN_ERR = 7,
    SERVER_STATUS_ERR = 8,
    SERVER_ACCEPT_ERR = 9,
    RECV_ERROR = 10,
};

class WebSocketServer{
    friend class HTTPServer;
private:
    HTTPServer& http_server_ref_;
public:
};

class WebSocketClient{
    friend class HTTPClient;
private:
    HTTPClient& http_client_ref_;
public:
};

class HTTPServer{
public:
    enum class Status{
        SOCKET_INVALID = 0,
        SOCKET_CREATED = 1,
        SOCKET_BIND = 2,
        SOCKET_LISTENING = 3,
    };
    enum class ConnType{
        HTTP_LONG = 0,
        WEBSOCKET = 2,
    };
private:
    Socket sock_;
    Status status_;
    std::unordered_map<std::string,std::function<void(HTTPRequest const&,HTTPResponse&)>> routes_;
    std::unordered_map<std::string,std::function<void(HTTPRequest const&,HTTPResponse&,std::smatch const&)>> matches_;
    std::unordered_map<std::string,std::string> statics_;
    std::unordered_set<std::string> websocket_routes_;
    std::unordered_map<Socket,ConnType,Socket::Hash> connections_;

    std::function<void(HTTPErrorType)> err_cb_=[](HTTPErrorType){};


    template<HTTPErrorType _et>void __rise_error();
    bool __expected_status(Status s);
public:
    HTTPServer();
    HTTPServer(IPAddr const&);
    HTTPServer(HTTPServer&&)noexcept;
    HTTPServer(HTTPServer const&)=delete;
    HTTPServer& operator=(HTTPServer&&)noexcept;
    HTTPServer& operator=(HTTPServer const&)=delete;
    ~HTTPServer();
    void set_server_addr(IPAddr const&) noexcept;
    bool listen();
    void close();

    void route_on(std::string const& route, std::function<void(HTTPRequest const&,HTTPResponse&)> const& cb) noexcept;
    void match_on(std::string const& reg,std::function<void(HTTPRequest const&,HTTPResponse&,std::smatch const&)> const& cb ) noexcept;
    void static_on(std::string const& route, std::string const& relative_path) noexcept;
    WebSocketServer upgrade_websocket(std::string const& route);
    void on_error(std::function<void(HTTPErrorType)> const& cb);
};

class HTTPClient{
private:
    Socket sock_;
    IPAddr addr_;
    bool connected_;
    std::function<void(HTTPErrorType)> err_cb_=[](HTTPErrorType){};
    template<HTTPErrorType _et>void __rise_error();
public:
    // will atomaticly call close function
    ~HTTPClient();
    HTTPClient();
    HTTPClient(HTTPClient&&) noexcept;
    HTTPClient(HTTPClient const&)=delete;
    HTTPClient& operator=(HTTPClient&&)noexcept;
    HTTPClient& operator=(HTTPClient const&)=delete;
    HTTPClient(IPAddr const&);
    void set_server_addr(IPAddr const&);
    std::optional<HTTPResponse> send(HTTPRequest const& req);
    void close();
    WebSocketClient upgrade_websocket(std::string const& route);
    void on_error(std::function<void(HTTPErrorType)> const& cb);
};





// impl

// global functions
std::pair<HTTPClient,std::optional<HTTPResponse>> HTTP_GET(Url const& url,std::unordered_map<std::string,std::string> && addtional_headers){
    HTTPClient client(tcx::IPAddr::url(url));
    tcx::HTTPRequest req;
    req.type = tcx::HTTPReqType::T_GET;
    req.headers = std::move(addtional_headers);
    req.route = url.route;
    std::pair<HTTPClient,std::optional<HTTPResponse>> ret;
    ret.first = std::move(client);
    ret.second = ret.first.send(req);
    return std::move(ret);
}
std::pair<HTTPClient,std::optional<HTTPResponse>> HTTP_POST(Url const& url, Blob && msg,std::unordered_map<std::string,std::string> && addtional_headers){
    HTTPClient client(tcx::IPAddr::url(url));
    tcx::HTTPRequest req;
    req.type = tcx::HTTPReqType::T_POST;
    req.headers = std::move(addtional_headers);
    if(!req.headers.count("Content-Length")){
        req.headers.emplace("Content-Length",std::to_string(msg.size()));
    }
    req.route = url.route;
    req.body = std::move(msg);

    std::pair<HTTPClient,std::optional<HTTPResponse>> ret;
    ret.first = std::move(client);
    ret.second = ret.first.send(req);
    return std::move(ret);
}
std::optional<HTTPResponse> HTTP_read_res_from_sock(Socket & sock_){
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

}
std::optional<HTTPRequest> HTTP_read_req_from_sock(Socket & sock_){
    // read
    std::optional<tcx::Blob> opt_blob = sock_.readuntill("\r\n\r\n");
    if(!opt_blob.has_value())return {};// read error
    std::optional<HTTPRequest> opt_req = HTTP_read_req(opt_blob.value());

    if(!opt_req.has_value())return {};
    HTTPRequest& req = opt_req.value();

    if(!sock_.readable()) return opt_req;// just head

    // read body
    if(req.headers.count("Content-Length")){
        int content_len = dec2i(req.headers["Content-Length"].c_str());// got content length
        if(content_len==-1 || content_len==0)return opt_req;// content len error or no content
        
        std::optional<tcx::Blob> res_opt2 = sock_.readall(content_len,std::chrono::milliseconds(1000));
        if(res_opt2.has_value()){
            req.body = std::move(res_opt2.value());
            return opt_req;
        }else return opt_req;// read error
    }else{
        read_body:
        std::optional<tcx::Blob> res_opt2 = sock_.readuntill("\r\n");
        if(!res_opt2.has_value()) return opt_req;// read error
        auto pos = res_opt2.value().find_first_of('\r');
        if(pos == std::string::npos)return opt_req;// content error
        int content_len = hex2i((char*)res_opt2.value().data(),(int)pos);
        if(content_len==-1 || content_len==0)return opt_req;// content len error or no content
        
        res_opt2 = sock_.readall(content_len,std::chrono::milliseconds(1000));
        if(res_opt2.has_value()){
            req.body << std::move(res_opt2.value());
            if(!sock_.readable()) return opt_req;// read finish
            else{
                res_opt2 = sock_.readuntill("\r\n");
                if(!res_opt2.has_value()) return opt_req;// read error
                goto read_body;
            }
        }else return opt_req;// read error
    }

}

// http server
template<HTTPErrorType _et>void  HTTPServer::__rise_error(){
    err_cb_(_et);
}
bool HTTPServer::__expected_status(Status s){
    if(this->status_!=s){
        __rise_error<HTTPErrorType::SERVER_STATUS_ERR>();
        return false;
    }
    return true;
}
HTTPServer::HTTPServer():sock_(Socket::safe_create(Socket::Protocol::TCP)){
    status_ = sock_.valid()? Status::SOCKET_CREATED:Status::SOCKET_INVALID;
}
HTTPServer::HTTPServer(HTTPServer&& other)noexcept
    :sock_(std::move(other.sock_))
    ,status_(other.status_)
    ,routes_(std::move(other.routes_))
    ,matches_(std::move(other.matches_))
    ,statics_(std::move(other.statics_))
    ,websocket_routes_(std::move(other.websocket_routes_))
    ,connections_(std::move(other.connections_))
    ,err_cb_(std::move(other.err_cb_))
{
    other.status_ = Status::SOCKET_INVALID;
}
HTTPServer& HTTPServer::operator=(HTTPServer&& other)noexcept{
    if(!sock_.close()) __rise_error<HTTPErrorType::UNKNOWN_ERR>();

    this->sock_=std::move(other.sock_);
    this->status_=other.status_;
    this->routes_=std::move(other.routes_);
    this->matches_=std::move(other.matches_);
    this->statics_=std::move(other.statics_);
    this->websocket_routes_=std::move(other.websocket_routes_);
    this->connections_=std::move(other.connections_);
    this->err_cb_=std::move(other.err_cb_);

    other.status_ = Status::SOCKET_INVALID;

    return *this;
}
HTTPServer::HTTPServer(IPAddr const& addr):sock_(Socket::safe_create(Socket::Protocol::TCP)){
    status_ = sock_.valid()? Status::SOCKET_CREATED:Status::SOCKET_INVALID;
    if(!__expected_status(Status::SOCKET_CREATED)) return;
    if(!sock_.bind(addr)){
        __rise_error<HTTPErrorType::ADDR_USED>();
        return;
    }
    status_ = Status::SOCKET_BIND;

}
void HTTPServer::set_server_addr(IPAddr const& addr) noexcept{
    if(!sock_.bind(addr)){
        __rise_error<HTTPErrorType::ADDR_USED>();
        return;
    }
    status_ = Status::SOCKET_BIND;
}
HTTPServer::~HTTPServer(){
    if(!sock_.close()) __rise_error<HTTPErrorType::UNKNOWN_ERR>();
}
void HTTPServer::close(){
    if(!sock_.close()) __rise_error<HTTPErrorType::UNKNOWN_ERR>();
}
bool HTTPServer::listen(){
    if(!__expected_status(Status::SOCKET_BIND)) return false;
    if(!sock_.listen()) {
        __rise_error<HTTPErrorType::UNKNOWN_ERR>();
        return false;
    }
    status_ = Status::SOCKET_LISTENING;


    while(status_ == Status::SOCKET_LISTENING){
        while(sock_.acceptable().value_or(false)){
            std::optional<tcx::Socket> client_opt = sock_.accept();
            if(!client_opt.has_value()){
                __rise_error<HTTPErrorType::SERVER_ACCEPT_ERR>();
                continue;
            }
            connections_.emplace(std::move(client_opt.value()),ConnType::HTTP_LONG);
        }

        for(auto i = connections_.begin();i!=connections_.end();){
            Socket& client = const_cast<Socket&>(i->first);

            if(i->second == ConnType::HTTP_LONG){
                if(!client.readable().value_or(false)){
                    if(client.has_error().value_or(true)){
                        std::cout << "error: "<<client.exact_err().value_or(-1)<<'\n';
                    }
                    goto next_client;
                }
                std::optional<HTTPRequest> reqopt = HTTP_read_req_from_sock(client);
                if(!reqopt.has_value()) {
                    __rise_error<HTTPErrorType::RECV_ERROR>();
                    goto next_client;
                }

                HTTPRequest& req = reqopt.value();
                HTTPResponse res;
                std::string& route = req.route;
                std::smatch sm;

                // route
                if(routes_.count(route)){
                    routes_[route](req,res);
                    goto finish_callback;
                }

                // static
                if(statics_.count(route)){
                    goto finish_callback;
                }

                // regex
                for(auto&j:matches_){
                    if(std::regex_match(route,sm,std::regex(j.first))){
                        j.second(req,res,sm);
                        goto finish_callback;
                    }
                }


                finish_callback:
                // send res
                if(!client.write_all(HTTP_to_blob(req),std::chrono::milliseconds(3000))){
                    __rise_error<HTTPErrorType::CONNECT_TIMEOUT>();
                }

                // keep or close
                if(req.headers.count("Connction")){
                    if(req.headers["Connection"] == "close"){
                        client.close();
                        goto erese_current_client;
                    }
                }
            }
            else if(i->second == ConnType::WEBSOCKET){

            }

            next_client:
            {
                ++i;
                continue;
            }
            erese_current_client:
            {
                i = connections_.erase(i);
                continue;
            }
        }
    }
    return true;
}
void HTTPServer::on_error(std::function<void(HTTPErrorType)> const& cb){
    err_cb_ = cb;
}
void HTTPServer::route_on(std::string const& route, std::function<void(HTTPRequest const&,HTTPResponse&)> const& cb) noexcept{
    if(!routes_.count(route)) routes_.emplace(route,cb);
    else routes_[route] = cb;
}
void HTTPServer::match_on(std::string const& reg,std::function<void(HTTPRequest const&,HTTPResponse&,std::smatch const&)> const& cb) noexcept{
    if(!matches_.count(reg)) matches_.emplace(reg,cb);
    else matches_[reg] = cb;
}
void HTTPServer::static_on(std::string const& route, std::string const& relative_path) noexcept{
    if(!statics_.count(route)) statics_.emplace(route,relative_path);
    else statics_[route] = relative_path;
}



// http client
template<HTTPErrorType _et>void  HTTPClient::__rise_error(){
    err_cb_(_et);
}
HTTPClient::~HTTPClient(){
    if(connected_){if(!sock_.close()) __rise_error<HTTPErrorType::UNKNOWN_ERR>();}
}
HTTPClient::HTTPClient(HTTPClient&& other)noexcept
    :sock_(std::move(other.sock_))
    ,connected_(other.connected_)
    ,addr_(std::move(other.addr_))
{
    other.connected_ = false;
}
HTTPClient& HTTPClient::operator=(HTTPClient&& other)noexcept{
    if(connected_){if(!sock_.close()) __rise_error<HTTPErrorType::UNKNOWN_ERR>();}
    this->sock_ = std::move(other.sock_);
    this->connected_ =other.connected_;
    this->addr_ =std::move(other.addr_);

    other.connected_ = false;

    return *this;
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
        return HTTP_read_res_from_sock(sock_);
    }else{
        int errorcode = sock_.fast_err();
        __rise_error<HTTPErrorType::SEND_ERROR>();
        return {};
    }
}
void HTTPClient::close(){
    if(!sock_.close()) __rise_error<HTTPErrorType::UNKNOWN_ERR>();
    connected_ = false;
}
void HTTPClient::on_error(std::function<void(HTTPErrorType)> const& cb){
    err_cb_ = cb;
}

}