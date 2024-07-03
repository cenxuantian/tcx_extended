#pragma once

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <string>
#include <type_traits>
#include <functional>

#ifdef _WIN32
#define __t_blob_msize(_ptr)  ::_msize((char*)_ptr)
#elif defined(__linux__)
#define __t_blob_msize(_ptr) ::malloc_usable_size(_ptr);
#else
static_assert(false,"platform not supported")
#endif

#define __my_requires(...) typename = typename std::enable_if_t<__VA_ARGS__>

namespace tcx
{
constexpr char strend = 0;

struct BlobShadow{
    unsigned char* buf_;
    unsigned long long capacity_;
    unsigned long long start_;
    unsigned long long size_;
};

class Blob
{
private:
    using __usize = unsigned long long;
    using __byte = unsigned char;
    __byte* buf_;
    __usize capacity_;
    __usize start_;
    __usize size_;

    void just_leak()noexcept{buf_ = 0;capacity_ = 0;start_=0;size_ = 0;}
    void release_anyway()noexcept{if(capacity_) ::free(buf_);just_leak();}
    inline static __usize next_capacity(__usize _capacity)noexcept{return (_capacity+1)*2;}
    static __usize capacity_until_fit(__usize _capacity,__usize target)noexcept{
        while (target > _capacity) 
            _capacity = next_capacity(_capacity);
        return _capacity;
    }

    void insert_data(const void* pointer, __usize size_of_pointer, __usize offset){
        __usize front_free_size = start_;
        __usize back_free_size = capacity_-size_-start_;
        __usize total_free_size = front_free_size+back_free_size;
        __usize after_data_size = size_+size_of_pointer;
        if(total_free_size>=size_of_pointer){
            if(front_free_size>=size_of_pointer){
                ::memmove(buf_+start_-size_of_pointer,buf_+start_,offset);
                start_=start_-size_of_pointer;
                ::memcpy(buf_+start_+offset,pointer,size_of_pointer);
                size_ = after_data_size;
            }else{
                __usize diff_size = size_of_pointer-start_;
                ::memmove(buf_,buf_+start_,offset);
                ::memmove(buf_+start_+offset+diff_size,buf_+start_+offset,size_-offset);
                ::memcpy(buf_+offset,pointer,size_of_pointer);
                start_=0;
                size_ = after_data_size;
            }
        }else{
            __usize target_capacity = capacity_until_fit(capacity_,after_data_size);
            __byte* new_buf = (__byte*)malloc(target_capacity);
            ::memcpy(new_buf,buf_+start_,offset);
            ::memcpy(new_buf+offset,pointer,size_of_pointer);
            ::memcpy(new_buf+offset+size_of_pointer,buf_+start_+offset,size_-offset);
            capacity_ = target_capacity;
            start_=0;
            size_ = after_data_size;
            free(buf_);
            buf_=new_buf;
        }
    }
    void overlap_data(const void* pointer, __usize size_of_pointer, __usize offset){
        __usize after_size = start_ + offset + size_of_pointer;
        if(after_size > capacity_){
            __usize real_data_size = offset+size_of_pointer;
            if(real_data_size <= capacity_){
                if(offset) ::memmove(buf_,buf_+start_,offset);
                start_=0;
                ::memcpy(buf_+offset,pointer,size_of_pointer);
                size_=real_data_size;
            }else{
                auto target_capacity = capacity_until_fit(capacity_,real_data_size);
                __byte* new_buf = (__byte*)malloc(target_capacity);
                if(offset)::memcpy(new_buf,buf_+start_,offset);
                ::memcpy(new_buf+offset,pointer,size_of_pointer);
                capacity_=target_capacity;
                start_=0;
                size_ = real_data_size;
                free(buf_);
                buf_=new_buf;
            }
        }else{
            ::memcpy(buf_+start_+offset,pointer,size_of_pointer);
            size_=after_size>size_?after_size:size_;
        }
    }

    // take over create
    Blob(__byte* _buf, __usize _capasity,__usize _start, __usize _end)noexcept:buf_(_buf),capacity_(_capasity),start_(_start),size_(_end){}

public:
    // deleted
    Blob(const Blob&) = delete;
    Blob& operator=(const Blob&) = delete;
    
    // cons & des
    Blob()noexcept:buf_(0),capacity_(0),start_(0),size_(0){}
    Blob(Blob&& _other)noexcept:buf_(_other.buf_),capacity_(_other.capacity_),start_(_other.start_),size_(_other.size_){_other.just_leak();}
    ~Blob(){release_anyway();}

    // operators
    Blob& operator=(Blob&& _other)noexcept{
        release_anyway();
        buf_=_other.buf_;
        size_=_other.size_;
        capacity_=_other.capacity_;
        start_=_other.start_;
        _other.just_leak();
        return*this;
    }
    __byte& operator[](__usize pos){return buf_[pos+start_];}
    __byte const& operator[](__usize pos)const{return buf_[pos+start_];}
    
    // auto sized push back
    template<typename T>
    Blob& operator<<(T&& data){
        overlap(std::forward<T&&>(data),size_);
        return *this;
    }
    // auto sized push front
    template<typename T>
    Blob& operator>>(T&& data){
        insert(std::forward<T&&>(data),0);
        return *this;
    }

    // read from front
    Blob& operator>(Blob& _other){
        _other = std::move(*this);
        return *this;
    }
    template<typename T, __my_requires((std::is_pointer_v<T>&&!std::is_same_v<const char*,T>))>
    Blob& operator>(T target_pointer){
        auto _size = __t_blob_msize(target_pointer);
        ::memcpy(target_pointer,buf_+start_,_size);
        pop_front(_size);
        return *this;
    }
    template<typename T, __my_requires(!std::is_pointer_v<T>)>
    Blob& operator>(T const& target){
        ::memcpy(&target,buf_+start_,sizeof(T));
        pop_front(sizeof(T));
        return *this;
    }

    // read from back
    Blob& operator<(Blob& _other){
        _other = std::move(*this);
        return *this;
    }
    template<typename T, __my_requires((std::is_pointer_v<T>&&!std::is_same_v<const char*,T>))>
    Blob& operator<(T target_pointer){
        auto _size = __t_blob_msize(target_pointer);
        ::memcpy(target_pointer,buf_+start_,_size);
        pop_back(_size);
        return *this;
    }
    template<typename T, __my_requires(!std::is_pointer_v<T>)>
    Blob& operator<(T const& target){
        ::memcpy(&target,buf_+start_,sizeof(T));
        pop_back(sizeof(T));
        return *this;
    }

    // funcs
    __byte const* data(__usize _offset = 0)const noexcept {return buf_+_offset+start_;}
    const char* c_str(__usize _offset = 0) const noexcept{
        const_cast<Blob*>(this)->operator<<((char)0);
        const_cast<Blob*>(this)->pop_back(1);
        return (char*)buf_+_offset+start_;
    }
    void* buf(__usize _offset = 0)noexcept{return (void*)(buf_+_offset+start_);}
    __byte const* original_data()const noexcept{return buf_;}
    void* original_buf()const noexcept{return (void*)buf_;}
    __usize capacity()const noexcept{return capacity_;}
    __usize size()const noexcept{return size_;}
    Blob completely_clone()const noexcept{
        __byte* buf = (__byte*)::malloc(capacity_);
        ::memcpy(buf,buf_,size_);
        return Blob(buf,capacity_,start_,size_);
    }
    Blob clone()const noexcept{
        __byte* buf = (__byte*)::malloc(size_);
        ::memcpy(buf,buf_,size_);return Blob(buf,size_,0,size_);
    }
    void shrink_to_fit()noexcept{
        __byte* new_buf = (__byte*)malloc(size_);
        memcpy(new_buf,buf_,size_);
        start_=0;
        free(buf_);
        buf_=new_buf;
    }
    void clear()noexcept{
        size_ = 0;
        start_ = 0;
    }
    // this functino only changes the capacity
    void reserve(__usize _capacity)noexcept{
        if(_capacity > capacity_){
            buf_ = (__byte*)realloc(buf_,_capacity);
            capacity_ = _capacity;
        }
    }
    // this function will change the size, may result some uncertain byte be caontained
    // will change the capacity if there is no enough space
    void resize(__usize _size){
        if(_size < size_) size_ = _size;
        else{
            if(start_+_size <=capacity_)size_ = _size;
            else {
                reserve(start_+_size);
                size_ = _size;
            }
        }
    }
    BlobShadow leak()noexcept{BlobShadow res={buf_,capacity_,start_,size_};just_leak();return res;}

    void overlap(char* pointer,__usize offset = 0){
        overlap_data(pointer,__t_blob_msize(pointer),offset);
    }
    void overlap(const char* pointer,__usize offset = 0){
        overlap_data((void*)pointer,strlen(pointer),offset);
    }
    void overlap(Blob const& _other,__usize offset = 0){
        overlap_data((void*)_other.data(),_other.size(),offset);
    }
    void overlap(Blob && _other,__usize offset = 0){
        if(offset == 0 && size_ ==0){
            this->operator=(std::move(_other));
        }else{
            overlap_data((void*)_other.data(),_other.size(),offset);
        }
    }
    void overlap(std::string const& _str, __usize offset = 0){
        overlap_data((void*)_str.data(),_str.size(),offset);
    }
    template<typename T, __my_requires(std::is_pointer_v<T>)>
    void overlap(T pointer,__usize offset = 0){
        overlap_data(pointer,__t_blob_msize(pointer),offset);
    }
    template<typename T, __my_requires(!std::is_pointer_v<T>)>
    void overlap(T const& not_pointer,__usize offset = 0){
        overlap_data((void*)&not_pointer,sizeof(not_pointer),offset);
    }


    void insert(char* pointer,__usize offset = 0){
        insert_data(pointer,__t_blob_msize(pointer),offset);
    }
    void insert(const char* pointer,__usize offset = 0){
        insert_data((void*)pointer,strlen(pointer),offset);
    }
    void insert(Blob const& _other,__usize offset = 0){
        insert_data((void*)_other.buf_,_other.size(),offset);
    }
    void insert(Blob && _other,__usize offset = 0){
        if(offset == 0 && size_ ==0){
            this->operator=(std::move(_other));
        }else{
            insert_data((void*)_other.buf_,_other.size(),offset);
        }
    }
    template<typename T, __my_requires(std::is_pointer_v<T>)>
    void insert(T pointer,__usize offset = 0){
        insert_data(pointer,__t_blob_msize(pointer),offset);
    }
    template<typename T, __my_requires(!std::is_pointer_v<T>)>
    void insert(T const& not_pointer,__usize offset = 0){insert_data((void*)&not_pointer,sizeof(not_pointer),offset);}

    __usize pop_back(__usize _size) noexcept{
        if(size_>=_size){
            size_-=_size;
            return _size;
        }{
            __usize res = size_;
            size_=0;
            start_=0;
            return res;
        }
    }
    __usize pop_front(__usize _size)noexcept{
        if(size_>=_size){
            start_+=_size;
            size_-=_size;
            return _size;
        }{
            __usize res = size_;
            size_=0;
            start_=0;
            return res;
        }
    }
    __usize read(void* target, __usize _size, __usize offset)noexcept{
        if(size_-offset>=_size){
            ::memcpy(target,buf_+offset+start_,_size);
            return _size;
        }else{
            ::memcpy(target,buf_+offset+start_,size_-offset);
            return size_-offset;
        }
    }

    void assign(const void* pointer, __usize size_of_pointer, __usize offset){overlap_data(pointer,size_of_pointer,offset);}
    void assign_back(const void* pointer, __usize size_of_pointer){overlap_data(pointer,size_of_pointer,start_);}
    void assign_front(const void* pointer, __usize size_of_pointer){insert_data(pointer,size_of_pointer,0);}

    template<typename T, typename FuncType,__my_requires(std::is_invocable_v<FuncType,std::decay_t<T> const&>)>
    void for_each(FuncType&& callback)const{
        for(__usize i=0;i<size_/sizeof(std::decay_t<T>);i++){
            std::decay_t<T> const* pos =(std::decay_t<T> const*) (buf_ + start_+ (i*sizeof(std::decay_t<T>)));
            callback(*pos);
        }
    }
    template<typename T, typename FuncType,__my_requires(std::is_invocable_v<FuncType,std::decay_t<T> &>)>
    void for_each(FuncType&& callback){
        for(__usize i=0;i<size_/sizeof(std::decay_t<T>);i++){
            std::decay_t<T>* pos =(std::decay_t<T>*) (buf_ + start_+ (i*sizeof(std::decay_t<T>)));
            callback(*pos);
        }
    }

    // find the first of c, start from the pos _offset(contained)
    // returns the c pos start from _offset if found
    // else return npos
    __usize find_first_of(unsigned char c,size_t _offset = 0)const{
        for(__usize i = start_+_offset; i < start_+_offset+size_;i++){
            if(this->buf_[i] == c)return i-start_-_offset;
        }
        return std::string::npos;
    }
    __usize find_first_of(std::string const& c,size_t _offset = 0)const{
        size_t sz = c.size();
        unsigned char* compare_buf = (unsigned char*)c.data();
        for(__usize i = start_+_offset; i < start_+_offset+size_-sz+1;i++){
            if(buf_[i] == compare_buf[0]){
                if(memcmp(buf_+i,compare_buf,sz) == 0) return i-start_-_offset;
            }
        }
        return std::string::npos;
    }
    __usize find_last_of(unsigned char c,size_t _offset = 0)const{
        for(__usize i = start_+_offset+size_-1; i >= start_+_offset;i--){
            if(this->buf_[i] == c)return i-start_-_offset;
        }
        return std::string::npos;
    }
    __usize find_first_not_of(unsigned char c,size_t _offset = 0)const{
        for(__usize i = start_+_offset; i < start_+_offset+size_;i++){
            if(this->buf_[i] != c)return i-start_-_offset;
        }
        return std::string::npos;
    }
    __usize find_last_not_of(unsigned char c,size_t _offset = 0)const{
        for(__usize i = start_+_offset+size_-1; i >= start_+_offset;i--){
            if(this->buf_[i] != c)return i-start_-_offset;
        }
        return std::string::npos;
    }
    
    
    // compare
    bool operator==(Blob const& other) const noexcept{
        if(size_!=other.size_)return false;
        return memcmp(data(),other.data(),size_)==0;
    }
    bool operator==(std::string const& other) const noexcept{
        if(size_!=other.size())return false;
        return memcmp(data(),other.data(),size_)==0;
    }
    bool operator==(const char* other) const noexcept{
        if(size_!=strlen(other))return false;
        return memcmp(data(),other,size_)==0;
    }
    bool operator!=(std::string const& other)const noexcept{
        return ! this->operator==(other);
    }
    bool operator!=(const char* other) const noexcept{
        return ! this->operator==(other);
    }
    bool operator!=(Blob const& other) const noexcept{
        return ! this->operator==(other);
    }

    // static
    static Blob take_over(void* _buf)noexcept {auto sz=__t_blob_msize(_buf); return Blob((__byte*)_buf,sz,0,sz);}

};


} // namespace tcx

#ifdef __t_blob_msize
#undef __t_blob_msize
#endif
#undef __my_requires
