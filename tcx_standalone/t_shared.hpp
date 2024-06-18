#pragma once

#ifdef __linux__
typedef void* HANDLE;
#include <errno.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#define SHM_SUBZONE_SIZE 65536
#endif


#include <string>
#include <optional>
#include <chrono>

namespace tcx
{
namespace{
inline int __get_last_error_no() {
#ifdef _WIN32
    return GetLastError();
#elif defined(__linux__)
    return errno;
#endif
}

unsigned long long _current_time_stamp() {
    auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    unsigned long long totalMs = now.time_since_epoch().count();
    return totalMs / 1000;
}

}
template<typename T,typename = std::enable_if_t<sizeof(T)<=SHM_SUBZONE_SIZE>>
class DeviceObject{
private:
    HANDLE _file_;
    void* _view_;
    int _last_error_;
   
    // create constructor
    template<typename...Args>
    DeviceObject(std::string const& name,Args&&...args)
#ifdef _WIN32
    :_file_(CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)sizeof(T), name.data()))
#endif
    ,_last_error_(INT_MIN)
    ,_view_(NULL)
    {
        if (_file_!=NULL) {
            _view_ = MapViewOfFile(_file_, FILE_MAP_ALL_ACCESS, 0, 0,(DWORD)sizeof(T));
            if(!_view_) {_last_error_ =  __get_last_error_no();; return;}
            auto _ = new (_view_) T(std::forward<Args&&>(args)...);
        }else _last_error_ =  __get_last_error_no();;
    }

    // read constructor
    DeviceObject(std::string const& name)
#ifdef _WIN32
    :_file_(CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)sizeof(T), name.data()))
#endif
    ,_last_error_(INT_MIN)
    ,_view_(NULL)
    {
        if (_file_!=NULL) {
            _view_ = MapViewOfFile(_file_, FILE_MAP_ALL_ACCESS, 0, 0,(DWORD)sizeof(T));
            if(!_view_) {_last_error_ =  __get_last_error_no();; return;}
        }
        else _last_error_ =  __get_last_error_no();;
    }

public:
    ~DeviceObject(){
#ifdef _WIN32
        if(_view_){UnmapViewOfFile(_view_);_view_ = nullptr;}
        if(_file_){ CloseHandle(_file_); _file_ = 0;}
#endif
    }

    DeviceObject(const DeviceObject&)=delete;
    DeviceObject(DeviceObject&& other)noexcept :_file_(other._file_),_view_(other._view_),_last_error_(other._last_error_){
        other._file_ = 0;
        other._view_ = 0;
        other._last_error_ = INT_MIN;
    };
    DeviceObject& operator=(DeviceObject const&)=delete;
    DeviceObject& operator=(DeviceObject&& other){
        _file_ = other._file_;
        _view_ = other._view_;
        _last_error_ = other._last_error_;
        other._file_ = 0;
        other._view_ = 0;
        other._last_error_ = INT_MIN;
        return *this;
    };

    template<typename ...Args, typename = std::enable_if_t<sizeof...(Args)>>
    static DeviceObject create(std::string const& name, Args&&...args){return DeviceObject(name,std::forward<Args&&>(args)...);}
    static DeviceObject open(std::string const& name){return DeviceObject(name);}
    std::optional<int> get_error() const{if( _last_error_ == INT_MIN)return {};else return {_last_error_};}
    operator bool()const noexcept{return _view_!=NULL;}
    bool valid()const noexcept{return _view_!=NULL;}
    T& operator*() &{return *(T*)_view_;}
    T const& operator*()  const&{return *(const T*)_view_;}
    T* get() & noexcept{return (T*)_view_;}
};

class DeviceMutex{
private:
    HANDLE _mtx_;
    int _last_error_;

public:
    DeviceMutex(std::string const& name):_mtx_(CreateMutexA(NULL, FALSE, name.data())),_last_error_(INT_MIN){}
    DeviceMutex(DeviceMutex const&) = delete;
    DeviceMutex(DeviceMutex&& other)noexcept:_mtx_(other._mtx_),_last_error_(other._last_error_){other._mtx_ = 0;_last_error_=INT_MIN;}
    DeviceMutex& operator=(DeviceMutex const&) = delete;
    DeviceMutex& operator=(DeviceMutex&& other) noexcept{_mtx_ = other._mtx_;_last_error_=other._last_error_;other._mtx_ = 0;_last_error_=INT_MIN; return *this;}
    ~DeviceMutex(){if(_mtx_) CloseHandle(_mtx_);}
    bool unlock(){return ReleaseMutex(_mtx_);}
    bool lock(DWORD timeout = INFINITE){
#ifdef _WIN32
        DWORD ret = WaitForSingleObject(_mtx_, timeout);
        if (ret == WAIT_TIMEOUT) return false;
        if (ret == WAIT_FAILED) {
            _last_error_ = __get_last_error_no();
            return false;
        }
        return true;
#elif defined(__linux__)
        return false;
#endif
    }
    bool try_lock(){return lock(0);}
    std::optional<int> get_error() const{if( _last_error_ == INT_MIN)return {};else return {_last_error_};}
    operator bool()const noexcept{return _mtx_!=NULL;}
    bool valid()const noexcept{return _mtx_!=NULL;}
};


} // namespace tcx
