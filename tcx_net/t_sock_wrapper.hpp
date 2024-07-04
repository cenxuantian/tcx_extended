/*

--> 
    This file is a wrapper class based on tmc_Socket.hpp, 
    which is in another repo of mine -- t_socket
    This file is a updated version
    This interfaces here are almost the same as they are in t_socket
    Just a few modifications
    Removed the error auto handler in the tmc_Socket, to improve the performace
    Tip: the auto error handler has been removed, so you will not get Result<>
    returned from the function and error message(call stacks or error line), you
    need to debug by your self if socket returns errors
                                                26.06.2024 ---- Cenxuan
*/

/*
MIT License

Copyright (c) 2024 Cenxuan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#ifdef _WIN32
#define MSG_NOSIGNAL 0
#define be64toh _bswap64
#define htobe64 _bswap64
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#elif defined(__linux__)
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SD_BOTH SHUT_RDWR
#define SOCKET int
#define closesocket(_sock) close(_sock)
#include <arpa/inet.h>
#include <errno.h>
#endif


#include "../tcx_standalone/t_blob.hpp"
#include "../tcx_standalone/t_url.hpp"
#include <utility>
#include <tuple>
#include <chrono>
#include <memory>
#include <string>
#include <optional>
#include <tuple>

namespace tcx
{
class IPAddr;
class Socket;

namespace{

class Env{
    void start(){
#ifdef _WIN32
        WSADATA wsadata;
        if(WSAStartup(MAKEWORD(2,2), &wsadata)!= 0){
            started = false;
        }else{
            started = true;
        }
#else
        started =true;
#endif
    }
    void stop(){
#ifdef _WIN32
        if(WSACleanup()){// error
        }else{
            started = false;
        }
#else
        started =false;
#endif
    }
    bool started = false;
    unsigned long long size = 0;
public:
    Env (){}
    ~Env(){
        if(started) stop();
    }

    void add(){
        if(size ==0){
            start();
        }
        size++;
    }
    void remove(){
        if(size == 1){
            stop();
        }
        size--;
    }
};

static Env global_socket_env;
template<int _opt> struct GetSockOptDetails{
    typedef void type;
    static const int level = -1;
    static const int optname = -1;
};
#define ROUTE_SOCK_OPT(_optname,_level,_type)\
template<> struct GetSockOptDetails<_optname>{\
    typedef _type type;\
    static const int level = _level;\
    static const int optname = _optname;}

ROUTE_SOCK_OPT(SO_BROADCAST,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_DONTROUTE,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_ERROR,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_KEEPALIVE,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_LINGER,SOL_SOCKET,linger);
ROUTE_SOCK_OPT(SO_OOBINLINE,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_RCVBUF,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_SNDBUF,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_RCVLOWAT,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_SNDLOWAT,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_RCVTIMEO,SOL_SOCKET,timeval);
ROUTE_SOCK_OPT(SO_SNDTIMEO,SOL_SOCKET,timeval);
ROUTE_SOCK_OPT(SO_REUSEADDR,SOL_SOCKET,int);
ROUTE_SOCK_OPT(SO_TYPE,SOL_SOCKET,int);

// ROUTE_SOCK_OPT(IP_HDRINCL,IPPROTO_IP,int);
// ROUTE_SOCK_OPT(IP_OPTINOS,IPPROTO_IP,int);
// ROUTE_SOCK_OPT(IP_TOS,IPPROTO_IP,int);
// ROUTE_SOCK_OPT(IP_TTL,IPPROTO_IP,int);

// ROUTE_SOCK_OPT(TCP_MAXSEG,IPPROTO_TCP,int);
ROUTE_SOCK_OPT(TCP_NODELAY,IPPROTO_TCP,int);
#undef ROUTE_SOCK_OPT

inline int sys_errno(){
#ifdef __linux__
    return errno;
#elif defined(_WIN32)
    return GetLastError();
#endif
}

}

class IPAddr{
    friend class Socket;
public:
    enum class Type: int;
public:
    enum class Type: int{
        V4 = 0,
        V6 = 1,
    };
private:
    sockaddr_in addr_in_;
    Type type_ = Type::V4;
public:
    static IPAddr v4(const char* host,u_short port){
        IPAddr res;
        res.addr_in_.sin_family = AF_INET;
        res.addr_in_.sin_port = port;
#ifdef _WIN32
        res.addr_in_.sin_addr.S_un.S_addr = inet_addr(host);
#elif defined(__linux__)
        res.addr_in_.sin_addr.s_addr =inet_addr(host);
#endif
        return res;
    }
    static IPAddr v4(sockaddr_in const& addr_in){
        IPAddr res;
        ::memcpy(&res.addr_in_,&addr_in,sizeof(sockaddr_in));
        return res;
    }
    static IPAddr url(Url const& url){
        global_socket_env.add();
        IPAddr res;
        unsigned long inaddr;
        struct sockaddr_in ad;
        struct hostent *hp;
        memset(&ad, 0, sizeof(ad));
        ad.sin_family = AF_INET;
        inaddr = inet_addr(url.host.c_str());
        if (inaddr != INADDR_NONE)
            memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
        else
        {
            hp = gethostbyname(url.host.c_str());
            if (hp == NULL)
                return IPAddr{};
            memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
        }
        ad.sin_port = htons(url.port);
        memcpy(&res.addr_in_,&ad,sizeof(ad));
        global_socket_env.remove();
        return res;
    }
    Type type()const noexcept{
        return type_;
    }
    std::string host()const{
        return ::inet_ntoa(addr_in_.sin_addr);
    }
    u_short port()const noexcept{
        return addr_in_.sin_port;
    }
};

class Socket{
public:
    enum class Protocol: int;
    enum class ShutdownType :int;
public:
    enum class Protocol: int{
        P_OTHER = -1,
        TCP = 0x00,
        UDP = 0x01,
    };
    enum class ShutdownType :int{
        SDT_RECV = 0,
        SDT_SEND = 1,
        SDT_BOTH = 2,
    };

private:
    bool valid_ = false;
    SOCKET h_sock_ = INVALID_SOCKET;
    Protocol protocol_;
    IPAddr addr_;

    Socket()noexcept {global_socket_env.add();}//hide
    Socket(Protocol _protocol):protocol_(_protocol) {//hide
        global_socket_env.add();
        switch (_protocol)
        {
        case Protocol::TCP:
            h_sock_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            break;
        case Protocol::UDP:
            h_sock_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            break;
        }
        if(h_sock_ != INVALID_SOCKET){
            this->valid_ = true;
        }
    }
    Socket(int af,int type, int _protocol):protocol_(Protocol::P_OTHER){
        global_socket_env.add();
        h_sock_ = ::socket(af, type, _protocol);
        if(h_sock_ != INVALID_SOCKET){
            this->valid_ = true;
        }
    }


    // -1 error, 0 ok, 1 timeout
    int __await_readable_from_start(bool wait_forever,std::chrono::system_clock::time_point const& start,std::chrono::milliseconds const& _timeout){
        // wait
        if(wait_forever) {
            if(!await_readable(_timeout).has_value()) return -1;
        }
        else{
            auto past_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
            auto left_time = _timeout<=past_time?std::chrono::milliseconds(0):_timeout-past_time;
            if(left_time != std::chrono::milliseconds(0)){
                auto wait_res = await_readable(left_time);
                if(!wait_res.has_value()) return -1;
                else if(!wait_res.value()) return 1; // timeout
            }
            else return 1;//timeout
        }
        return 0;
    }

    // this func will call ::send or ::sendto
    // param buf content to send
    // param _fn function to call (send / sendto)
    // param args if sendto, the target
    template<typename _Fn,typename ..._Args>
    std::optional<int> __write(Blob const& buf,_Fn &&_fn, _Args &&...args){
        int ret = _fn(h_sock_,(const char*)buf.data(),(int)buf.size(),MSG_NOSIGNAL,std::forward<_Args>(args)...);
        if(ret==SOCKET_ERROR) return {};
        return ret;
    }

    // this func will call ::recv or ::recvfrom
    // param buf content to send
    // param _fn function to call (recv / recvfrom)
    // param args if recvfrom, the target
    template<typename _Fn,typename ..._Args>
    std::optional<Blob> __readsome(int _expect_size,_Fn &&_fn, _Args &&...args){
        char* buf = (char*)malloc(_expect_size);
        int ret = _fn(h_sock_,buf,_expect_size,0,std::forward<_Args>(args)...);
        if(ret == SOCKET_ERROR){
            free(buf);
            return {};
        }
        auto r = Blob::take_over(buf);
        r.resize(ret);
        return std::move(r);
    }


    // this func will call ::send or ::sendto
    // param buf content to send
    // param _fn function to call (send / sendto)
    // param args if sendto, the target
    // make sure all buf has been write
    template<typename _Fn,typename ..._Args>
    bool __write_all(std::chrono::milliseconds const& _timeout,Blob const& buf,_Fn &&_fn, _Args &&...args){
        auto start = std::chrono::system_clock::now();
        auto write_buf_size_res = get_write_bufsize();
        if(!write_buf_size_res.has_value())return false; // no write buf size
        int write_buf_size = write_buf_size_res.value();
        const char* content = (const char*)buf.data();
        int offset = 0;
        int left_size = buf.size();
        bool wait_forever = _timeout == std::chrono::milliseconds(0)? true:false;

        try_send:
        if(left_size<=0)return true;
        int this_time_write_size =std::min(left_size,write_buf_size);

        // wait
        if(wait_forever) {
            if(!await_writeable(_timeout).has_value()) return false;
        }
        else{
            auto past_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
            auto left_time = _timeout<=past_time?std::chrono::milliseconds(0):_timeout-past_time;
            if(left_time != std::chrono::milliseconds(0)){
                auto wait_res = await_writeable(left_time);
                if(!wait_res.has_value()) return false;
                else if(!wait_res.value()) return false;// currently con't read
            }else return false;//await time out
        }

        // write
        int ret = ret = _fn(h_sock_,content+offset,this_time_write_size,MSG_NOSIGNAL,std::forward<_Args>(args)...);
        if(ret == SOCKET_ERROR)return false;
        else if(left_size == ret) return true;
        offset+=ret;
        left_size-=ret;
        goto try_send;
        return false;
    }

    // this func will call ::recv or ::recvfrom
    // param buf content to send
    // param _fn function to call (recv / recvfrom)
    // param args if recvfrom, the target
    // make sure all buf has been read
    // will block if not enough
    template<typename _Fn,typename ..._Args>
    std::optional<Blob> __readall(std::chrono::milliseconds const& _timeout,int _size,_Fn &&_fn, _Args &&...args){
        auto start = std::chrono::system_clock::now();
        auto read_buf_size_res = get_read_bufsize();
        if(!read_buf_size_res.has_value()) return {};// no buf size

        bool wait_forever = _timeout == std::chrono::milliseconds(0)? true:false;
        int read_buf_size = read_buf_size_res.value();
        int total_read_size = 0;
        Blob blob;
        blob.reserve(_size);
        int left_size = _size;

        

        try_read:
        // prepare
        if(left_size<=0){
            blob.resize(total_read_size);
            return blob;// read finish
        }
        int this_time_read_size = std::min(left_size,read_buf_size);

        // wait
        if(wait_forever) {
            if(!await_readable(_timeout).has_value()) return {};
        }
        else{
            auto past_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
            auto left_time = _timeout<=past_time?std::chrono::milliseconds(0):_timeout-past_time;
            if(left_time != std::chrono::milliseconds(0)){
                auto wait_res = await_readable(left_time);
                if(!wait_res.has_value()) return {};
                else if(!wait_res.value()) {// currently con't read
                    blob.resize(total_read_size);
                    blob.pop_back(left_size); 
                    return blob;
                }
            }
            else {blob.pop_back(left_size); return blob;}//await time out
        }
        
        // read
        int ret =  _fn(h_sock_,(char*)blob.buf(_size-left_size),this_time_read_size,0,std::forward<_Args>(args)...);
        if(ret == SOCKET_ERROR) return {};// error
        left_size-=ret;
        total_read_size+=ret;
        goto try_read;
        return {};
    }


    template<typename _Fn,typename ..._Args>
    std::optional<Blob> __readuntil(std::chrono::milliseconds const& _timeout,Blob const& target,_Fn &&_fn, _Args &&...args){
        auto start = std::chrono::system_clock::now();
        auto read_buf_size_res = get_read_bufsize();
        if(!read_buf_size_res.has_value()) return {};// no buf size

        bool wait_forever = _timeout == std::chrono::milliseconds(0)? true:false;
        int read_buf_size = read_buf_size_res.value();
        int total_read_size = 0;
        Blob blob;
        int wait_ret;

        try_read:

        // wait
        wait_ret = __await_readable_from_start(wait_forever,start,_timeout);
        if(wait_ret == -1) return {};   // error
        else if(wait_ret == 1) return blob;// timeout

        char c=0;
        int ret = _fn(h_sock_,&c,1,0,std::forward<_Args>(args)...);
        if(ret == SOCKET_ERROR) return {};// error
        blob<<c;
        if(c == target[0]){
            int _left = target.size() - 1;
            int offset = blob.size();
            blob.resize(blob.size() + _left);
            // wait
            wait_ret = __await_readable_from_start(wait_forever,start,_timeout);
            if(wait_ret == -1) return {};   // error
            else if(wait_ret == 1) return blob;// timeout
            
            ret = _fn(h_sock_,(char*)blob.data(offset),_left,0,std::forward<_Args>(args)...);
            if(ret == SOCKET_ERROR) return {};// error
            if(memcmp(blob.data(offset),target.data(1),_left) == 0){
                return blob;// read finish
            }
        }
        goto try_read;
        return {};
    }

public:

// ---- create ---- 
    ~Socket(){
        this->close();
        global_socket_env.remove();
    }

    // create a socket
    static std::optional<Socket> create(Protocol _protocol){
        Socket res(_protocol);
        if(res.valid_) return {std::move(res)};
        else return {};
    }

    static Socket safe_create(Protocol _protocol){
        return Socket(_protocol);
    }
    
    // create a socket in native style
    static std::optional<Socket> create(int af,int type, int _protocol){
        Socket res(af,type,_protocol);
        if(res.valid_) return {std::move(res)};
        else return {};
    }


// ---- error ----
    // check if the socket is valid currently
    bool valid()const noexcept{return this->valid_;}
    // get the errno on linux or the result of GetLastError on windows
    // this number is always the last error of the system
    // please use exact_err instead when use multi-thread scheme
    static int fast_err(){return sys_errno();}
    // get the error number on this socket
    std::optional<int> exact_err(){return getopt<SO_ERROR>();}
    // rise an error that can be get by exact_err()
    bool rise_err(int error_number){return setopt<SO_ERROR>(error_number);}
    // use select to check if has error
    // Result can be ignored, will return false on error
    std::optional<bool> has_error(){
        timeval _tval;
        _tval.tv_sec = 0;
        _tval.tv_usec = 0;
        fd_set set;
        FD_ZERO(&set);
        FD_SET(h_sock_,&set);
        if(::select((int)h_sock_+1,0,0,&set,&_tval) == SOCKET_ERROR){
           return {};
        }
        if(FD_ISSET(h_sock_,&set)){// error
            return true;
        }else{  // no error
            return false;
        }
    }

// ---- information ----
    // get the native socket handle 
    SOCKET native_handle()const noexcept{return h_sock_;}
    // get the recorded address if called bind
    IPAddr const& address()const noexcept{return this->addr_;}
    // get the protocol
    Protocol protocol()const noexcept{return this->protocol_;}


// ---- operation ----
    // socet connect function
    // only valid for tcp
    bool connect(IPAddr const& addr){
        int ret = ::connect(h_sock_,(sockaddr*)&addr.addr_in_,sizeof(sockaddr_in));
        
        if(ret!= SOCKET_ERROR){
            sockaddr_storage storage;
#ifdef __linux__
            socklen_t sock_len = sizeof(sockaddr_storage);
#elif defined(_WIN32)
            int sock_len = sizeof(sockaddr_storage);
#endif
            ret = ::getsockname(h_sock_,(sockaddr*)&storage,&sock_len);
            if(ret == SOCKET_ERROR){
                return false;
            }else{
                this->addr_ = IPAddr::v4(*((sockaddr_in*)&storage));
                return true;
            }
        }else{
            return false;
        }
    }
    // socket accept function
    std::optional<Socket> accept(){
        Socket res(this->protocol_);
        sockaddr_in addr;
#ifdef __linux__
        socklen_t addr_len = sizeof(sockaddr_in);
#elif defined(_WIN32)
        int addr_len = sizeof(sockaddr_in);
#endif
        res.h_sock_ =  ::accept(h_sock_,(sockaddr*)&addr,&addr_len);
        if(res.h_sock_ != INVALID_SOCKET){
            res.valid_ = true;
        }
        memcpy(&(res.addr_.addr_in_),&addr,sizeof(sockaddr_in));
        if(res.addr_.addr_in_.sin_family == AF_INET){
            res.addr_.type_ = IPAddr::Type::V4;
        }else if(res.addr_.addr_in_.sin_family == AF_INET6){
            res.addr_.type_ = IPAddr::Type::V6;
        }
        return res.valid_? std::optional<Socket>{res}:std::optional<Socket>{};
    }
    // socet sut down function
    bool shutdown(ShutdownType type = ShutdownType::SDT_BOTH){return ::shutdown(h_sock_,(int)type)!=SOCKET_ERROR;}
    // socet close function
    bool close(){
        int ret = ::closesocket(h_sock_);
        if(ret!=SOCKET_ERROR){
            this->valid_ = false;
        }
        return ret!=SOCKET_ERROR;
    }
    // socket bind function
    bool bind(IPAddr const& addr){
        bool success = ::bind(h_sock_,(sockaddr*)&addr.addr_in_,sizeof(sockaddr_in)) != SOCKET_ERROR;
        this->addr_ = addr;
        return success;
    }
    // socket listen function
    bool listen(){ return ::listen(h_sock_,1)!=SOCKET_ERROR;}
    // socket send funtion
    // return size writen
    // param 0 message to write    
    std::optional<int> write(Blob const& buf){return __write(buf,::send);}
    // make sure write all the buffer content
    bool write_all(Blob const& buf,std::chrono::milliseconds const& _timeout = std::chrono::milliseconds(0)){return __write_all(_timeout,buf,::send);}
    // socket sendto funtion
    // return size writen
    // param 0 message to write
    // param 1 target of udp
    std::optional<int> write_to(Blob const& buf,IPAddr const& tar){return __write(buf,::sendto,(sockaddr*)&tar.addr_in_,(int)sizeof(sockaddr_in));}
    // make sure write all the buffer content
    bool write_all_to(Blob const& buf, IPAddr const& tar,std::chrono::milliseconds const& _timeout = std::chrono::milliseconds(0)){return __write_all(_timeout,buf,::sendto,(sockaddr*)&tar.addr_in_,(int)sizeof(sockaddr_in));}
    // socket recv function
    // param 0 size to read
    // if data in read buf is not enough will return ok(readsize)
    std::optional<Blob> readsome(int _expect_size){return __readsome(_expect_size,::recv);}
    std::optional<Blob> readall(int _expect_size,std::chrono::milliseconds const& _timeout = std::chrono::milliseconds(0)){return __readall(_timeout,_expect_size,::recv);}
    // the result will contain the target
    std::optional<Blob> readuntill(Blob const& target,std::chrono::milliseconds const& _timeout = std::chrono::milliseconds(0)){
        return __readuntil(_timeout,target,::recv);
    }
    // the result will contain the target
    std::optional<Blob> readuntill(const char* target,std::chrono::milliseconds const& _timeout = std::chrono::milliseconds(0)){
        Blob b;
        b<< target;
        return __readuntil(_timeout,b,::recv);
    }
    // socet recvfrom function
    // param 0 size to read
    // param 1 target of udp
    // if data in read buf is not enough will return ok(readsize)
    std::optional<Blob> readsome_from(int _expect_size, IPAddr const& tar){
#ifdef __linux__
        socklen_t addr_len = sizeof(sockaddr_in);
#elif defined(_WIN32)
        int addr_len = sizeof(sockaddr_in);
#endif
        return __readsome(_expect_size,::recvfrom,(sockaddr*)&tar.addr_in_,&addr_len);
    }
    std::optional<Blob> readall_from(int _expect_size, IPAddr const& tar,std::chrono::milliseconds const& _timeout = std::chrono::milliseconds(0)){
#ifdef __linux__
        socklen_t addr_len = sizeof(sockaddr_in);
#elif defined(_WIN32)
        int addr_len = sizeof(sockaddr_in);
#endif
        return __readall(_timeout,_expect_size,::recvfrom,(sockaddr*)&tar.addr_in_,&addr_len);
    }
    std::optional<Blob> readuntill_from(Blob const& target,IPAddr const& taraddr,std::chrono::milliseconds const& _timeout= std::chrono::milliseconds(0)){
#ifdef __linux__
        socklen_t addr_len = sizeof(sockaddr_in);
#elif defined(_WIN32)
        int addr_len = sizeof(sockaddr_in);
#endif
        return __readuntil(_timeout,target,::recvfrom,(sockaddr*)&taraddr.addr_in_,&addr_len);
    }
    // socket select function
    // return 0 error
    // return 1 recv
    // return 2 send
    std::optional<std::tuple<bool,bool,bool>> select(std::chrono::milliseconds const& _timeout){
        std::tuple<bool,bool,bool> res = {false,false,false};
        auto _ms = _timeout.count();
        timeval _tval;
        _tval.tv_sec = (decltype(timeval::tv_sec))_ms/1000;
        _tval.tv_usec = (decltype(timeval::tv_usec))_ms%1000;
        fd_set _recv_set;
        fd_set _send_set;
        fd_set _err_set;
        FD_ZERO(&_recv_set);
        FD_ZERO(&_send_set);
        FD_ZERO(&_err_set);
        FD_SET(h_sock_,&_recv_set);
        FD_SET(h_sock_,&_send_set);
        FD_SET(h_sock_,&_err_set);
        int _ret = ::select((int)h_sock_+1,&_recv_set,&_send_set,&_err_set,&_tval);
        if(_ret == SOCKET_ERROR) return {};
        else{
            if(FD_ISSET(h_sock_,&_recv_set)){// read
                std::get<1>(res) = true;
            }
            if(FD_ISSET(h_sock_,&_send_set)){
                std::get<2>(res) = true;
            }
            if(FD_ISSET(h_sock_,&_err_set)){
                std::get<0>(res) = true;
            }
            return res;
        }
    }
    // use select to check if the read buf is currently avaliable
    // Result can be ignored, will return false on error
    std::optional<bool> readable(){
        timeval _tval;
        _tval.tv_sec = 0;
        _tval.tv_usec = 0;
        fd_set set;
        fd_set _err_set;
        FD_ZERO(&set);
        FD_ZERO(&_err_set);
        FD_SET(h_sock_,&set);
        FD_SET(h_sock_,&_err_set);
        if(::select((int)h_sock_+1,&set,0,&_err_set,&_tval) == SOCKET_ERROR) return {};
        if(FD_ISSET(h_sock_,&_err_set)){
            if(FD_ISSET(h_sock_,&set))return {};
            else return {};
        }else{
            if(FD_ISSET(h_sock_,&set))return true;
            else return false;
        }
    }
    // use select to check if the write buf is currently avaliable
    // Result can be ignored, will return false on error
    std::optional<bool> writeable(){
        timeval _tval;
        _tval.tv_sec = 0;
        _tval.tv_usec = 0;
        fd_set set;
        fd_set _err_set;
        FD_ZERO(&set);
        FD_ZERO(&_err_set);
        FD_SET(h_sock_,&set);
        FD_SET(h_sock_,&_err_set);
        if(::select((int)h_sock_+1,0,&set,&_err_set,&_tval) == SOCKET_ERROR)return {};
        if(FD_ISSET(h_sock_,&_err_set)){
            if(FD_ISSET(h_sock_,&set))return {};
            else return {};
        }else{
            if(FD_ISSET(h_sock_,&set))return true;
            else return false;
        }
    }
    // wait until sock is readable
    // return true if readable after timeout
    // return false if readable after timeout
    // return {} 
    // if timeout is 0 wait forever
    // if you do not want to wait, please use readable
    std::optional<bool> await_readable(std::chrono::milliseconds const& timeout){
        if(timeout == std::chrono::milliseconds(0)){
            // wait forever
            try_readable:{
                std::optional<bool> readable_res = readable();
                if(readable_res.has_value()){
                    if(readable_res.value()){
                        return true;
                    }else{
                        goto try_readable;
                    }
                }else{
                    return {};
                }
            }
        }else{
            size_t count = timeout.count();
            timeval _tval;
            _tval.tv_sec = (int)count/1000;
            _tval.tv_usec = (int)count%1000;
            fd_set set;
            FD_ZERO(&set);
            FD_SET(h_sock_,&set);
            if(::select((int)h_sock_+1,&set,0,0,&_tval) == SOCKET_ERROR){
                return {};
            }
            if(FD_ISSET(h_sock_,&set)){// read
                return true;
            }else{
                return false;
            }
        }
    }
    // wait until sock is writeable
    // return ok(true) if writeable after timeout
    // return ok(false) if writeable after timeout
    // return err(false) 
    // if timeout is 0 wait forever
    // if you do not want to wait, please use writeable
    std::optional<bool> await_writeable(std::chrono::milliseconds const& timeout){
        if(timeout == std::chrono::milliseconds(0)){
            // wait forever
            try_writeable:{
                std::optional<bool> writeable_res = writeable();
                if(writeable_res.has_value()){
                    if(writeable_res.value()) return true;
                    else goto try_writeable;
                }
                else return {};
            }
        }else{
            size_t count = timeout.count();
            timeval _tval;
            _tval.tv_sec = (int)count/1000;
            _tval.tv_usec = (int)count%1000;
            fd_set set;
            FD_ZERO(&set);
            FD_SET(h_sock_,&set);
            if(::select((int)h_sock_+1,0,&set,0,&_tval) == SOCKET_ERROR){
                return {};
            }
            if(FD_ISSET(h_sock_,&set)){// read
                return true;
            }else{
                return false;
            }
        }
    }
    // check if there is pending connections
    // Result can be ignored, will return false on error
    std::optional<bool> acceptable(){return readable();}
    
// ---- option ----
    // get set
    template<int _opt>
    bool setopt(typename GetSockOptDetails<_opt>::type const& val){
        return ::setsockopt(
            h_sock_,
            GetSockOptDetails<_opt>::level,
            _opt,
            (const char*)&val,
            sizeof(typename GetSockOptDetails<_opt>::type)) !=SOCKET_ERROR;
    }
    template<int _opt>
    std::optional<typename GetSockOptDetails<_opt>::type> getopt(){
        typename GetSockOptDetails<_opt>::type data;
#ifdef __linux__
        socklen_t len = sizeof(typename GetSockOptDetails<_opt>::type);
#elif defined(_WIN32)
        int len = sizeof(typename GetSockOptDetails<_opt>::type);
#endif
        int ret = ::getsockopt(
            h_sock_,
            GetSockOptDetails<_opt>::level,
            _opt,
            (char*)&data,
            &len);
        if(ret==SOCKET_ERROR)return {};
        else return std::move(data);
    }
    
    // timeout
    bool set_write_timeout(std::chrono::milliseconds const& timeout){
        timeval val;
        size_t ms = timeout.count();
        val.tv_sec = (decltype(timeval::tv_sec))ms/1000;
        val.tv_usec = (decltype(timeval::tv_usec))ms%1000;
        return setopt<SO_SNDTIMEO>(val);
    }
    std::optional<std::chrono::milliseconds> get_write_timeout(){
        auto res = getopt<SO_SNDTIMEO>();
        if(res.has_value()){
            timeval val = res.value();
            return std::chrono::milliseconds((val.tv_sec*1000)+val.tv_usec);
        }else return {};
    }
    bool set_read_timeout(std::chrono::milliseconds const& timeout){
        timeval val;
        size_t ms = timeout.count();
        val.tv_sec = (decltype(timeval::tv_sec))ms/1000;
        val.tv_usec = (decltype(timeval::tv_usec))ms%1000;
        return setopt<SO_RCVTIMEO>(val);
    }
    std::optional<std::chrono::milliseconds> get_read_timeout(){
        auto res = getopt<SO_RCVTIMEO>();
        if(res.has_value()){
            timeval val = res.value();
            return std::chrono::milliseconds((val.tv_sec*1000)+val.tv_usec);
        }else return {};
    }
    
    // buf size
    bool set_write_bufsize(int bufsize){return setopt<SO_SNDBUF>(bufsize);}
    bool set_read_bufsize(int bufsize){return setopt<SO_RCVBUF>(bufsize);}
    std::optional<int> get_write_bufsize(){return getopt<SO_SNDBUF>();}
    std::optional<int> get_read_bufsize(){return getopt<SO_RCVBUF>();}

    // sensitivity
    bool set_write_sensitivity(int bytesize){return setopt<SO_SNDLOWAT>(bytesize);}
    bool set_read_sensitivity(int bytesize){return setopt<SO_RCVLOWAT>(bytesize);}
    std::optional<int> get_write_sensitivity(){return getopt<SO_SNDLOWAT>();}
    std::optional<int> get_read_sensitivity(){return getopt<SO_RCVLOWAT>();}
    
    // set close type via SO_LINGER
    // param _wait
    // _wait == -1  wait forever
    // _wait >=0    wait for _wait second(s)
    bool set_close_wait(int _wait){
        linger _l;
        if(_wait == -1){
            _l.l_onoff = 0;
        }else{
            _l.l_onoff = 1;
            _l.l_linger = _wait;
        }
        return setopt<SO_LINGER>(_l);
    }
    // get close type via SO_LINGER
    // return
    // == -1    wait forever
    // >=0      wait for _wait second(s)
    std::optional<int> get_close_wait(){
        auto res = getopt<SO_LINGER>();
        if(!res.has_value()) return {};
        int _wait;
        auto& _l = res.value();
        if(_l.l_onoff){
            _wait = _l.l_linger;
        }else{
            _wait = -1;
        }
        return _wait;
    }
    
    // reuseaddr
    bool set_reuse_addr(bool _reuseable){return setopt<SO_REUSEADDR>((int)_reuseable);}
    std::optional<bool> get_reuse_addr(){
        auto res = getopt<SO_REUSEADDR>();
        if(!res.has_value())return {};
        else return {(bool)res.value()};
    }

    // borad cast
    bool enable_broadcast(){return setopt<SO_BROADCAST>(1);}
    bool disable_broadcast(){return setopt<SO_BROADCAST>(0);}
    std::optional<bool> is_broadcast_enabled(){
        auto res = getopt<SO_BROADCAST>();
        if(!res.has_value())return {};
        else return {(bool)res.value()};
    }

};


} // namespace tcx
