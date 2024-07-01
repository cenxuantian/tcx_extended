#pragma once
#include <thread>
#include <unordered_set>
#include <functional>
#include <chrono>
#include <memory>
#include <future>
#include <queue>
#include <coroutine>
#include <map>
#include <iostream>

namespace tcx{
namespace sim{

struct Sus;
class Coro;
class Object;
class EnvRef;
class Env;
class Evnet;


class Object{
public:
    friend class Env;
private:
    // this function will call the start()
    virtual void  __start() final;
    virtual void __resume() final;
    virtual void __release() final;
protected:
    // the coroutine of running start() function
    Coro* pco_=nullptr;
public:
    Env* env=nullptr;
    Object()=default;
    Object(Object const&)=delete;
    Object(Object&&)=default;
    Object& operator=(Object const&)=delete;
    Object& operator=(Object&&)=default;
    virtual ~Object() = 0;
    virtual Coro start() = 0;
    virtual void stop() = 0;
    virtual void tick(size_t step_) = 0;

};

class Event{
    friend class Env;
    friend class EnvRef;
private:
    bool done_;
    size_t step_;
    std::function<void()> func_;
    bool operator==(size_t num)const noexcept;
    template<typename _Fn,typename ... _Args>
    Event(size_t _step,_Fn &&f, _Args &&...args);
public:
    bool operator<(Event const& other)const noexcept;
    bool done()const noexcept;
    void trigger();
};

class Env final{
    friend class Object;
    friend class EnvRef;
private:
    enum class Status{
        STOPPED = 0,
        STARTED = 1,
        STOPPING = 2,
    };
    enum class LogLevel{
        NORMAL = 0,
        DEBUG = 1,
        WARNING = 2,
        USER_ERROR = 3,
        ENV_ERROR = 4,
        SYS_ERROR = 5,
    };
    Status status_ = Status::STOPPED;
    size_t step_ = 0;
    std::unordered_set<Object*> objs_;
    std::map<size_t,std::unordered_set<Object*>> corotasks_;
    std::priority_queue<Event> event_queue_;
    template<LogLevel level,typename ...Args> inline constexpr void __log(const char* fmt, Args&& ...args)const;
    template<typename ...Args> inline constexpr void sys_err(const char* fmt, Args&& ...args) const;
    template<typename ...Args> inline constexpr void env_err(const char* fmt, Args&& ...args) const;
public:
    Env();
    ~Env();
    // run the environment
    void run(size_t max_step = std::string::npos);
    // stop the env
    void stop();
    // get current step
    size_t now() const noexcept;
    // logs
    template<typename ...Args> inline constexpr void log(const char* fmt, Args&& ...args) const;
    template<typename ...Args> inline constexpr void logd(const char* fmt, Args&& ...args) const;
    template<typename ...Args> inline constexpr void warn(const char* fmt, Args&& ...args) const;
    template<typename ...Args> inline constexpr void err(const char* fmt, Args&& ...args) const;
    // sleep for single item
    Sus sleep(Object* tar,size_t steps);
    // add asyn event
    template<typename _Fn,typename ... _Args>
    void set_timeout(_Fn &&f,size_t _time_out, _Args &&...args);
    // get obj list
    std::unordered_set<Object*>& obj_list();
    // create
    template<typename T, typename ...Args>
    requires(std::is_base_of_v<Object,T>)
    T* emplace_obj(Args&& ...args);
    template<typename T>
    requires(std::is_base_of_v<Object,T>)
    void erase_obj(T* obj);
};

struct Sus final{
private:
    const size_t steps_;
public:
    Sus(size_t wait_step):steps_(wait_step){}
    constexpr bool await_ready() const noexcept {return steps_==0;}
    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};

class Coro final{
public:
    struct promise_type{
        Coro get_return_object(){
            return Coro(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        auto initial_suspend()noexcept {return std::suspend_never{};};
        auto final_suspend()noexcept {return std::suspend_always{};};
        auto yield_value()noexcept{return std::suspend_never{};}
        void return_value() noexcept{}
        void unhandled_exception(){}
    };

    std::coroutine_handle<promise_type> handle;
    explicit Coro(std::coroutine_handle<promise_type> const& h):handle(h){}
    ~Coro(){handle.destroy();}
    void resume(){handle.resume();}
};


// ----------------- impl -----------------------------

// Event
template<typename _Fn,typename ... _Args>
Event::Event(size_t _step,_Fn &&f, _Args &&...args){
    std::function<decltype(f(args...))()> func = std::bind(std::forward<_Fn>(f), std::forward<_Args>(args)...);
    auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
    this->func_ = [task_ptr]() {(*task_ptr)();};
    this->step_ = _step;
    this->done_ = false;
}
bool Event::operator<(Event const& other)const noexcept{
    return step_>other.step_;
}
bool Event::operator==(size_t num)const noexcept{
    return step_==num;
}
bool Event::done()const noexcept{
    return done_;
}
void Event::trigger(){
    func_();
    done_=true;
}


// Env
Env::Env(){

}
Env::~Env(){
    for(auto& i:objs_){
        i->__release();
        delete i;
    }
}
size_t Env::now() const noexcept{
    return step_;
}
Sus Env::sleep(Object* tar,size_t steps){
    if(steps != 0){
        if(corotasks_.count(steps+step_)){
            corotasks_[steps+step_].emplace(tar);
        }else{
            corotasks_.emplace(steps+step_,std::unordered_set<Object*>{tar});
        }
    }else{
        warn("in function: %s if(steps == 0) will directly launch code below."
            "Please modify your code for better performance.",__FUNCTION__);
    }
    return Sus{steps};
}
std::unordered_set<Object*>& Env::obj_list(){
    return this->objs_;
}
void Env::run(size_t max_step){
    if(this->status_==Status::STOPPED){
        for(auto& i : objs_){
            if(!i){
                env_err("there is an nullptr in the objs_ of Env(addr.=%X)",this);
                continue;
            }
            i->__start();
        }
        status_ = Status::STARTED;

        while(status_ == Status::STARTED && (max_step == std::string::npos || step_<=max_step)){
            if(corotasks_.count(step_)){
                for(Object* each: corotasks_[step_]){
                    if(!each)env_err("there is an nullptr in the objs_ of Env(addr.=%X)",this);
                    else if(!each->pco_)env_err("in Env(addr.=%X), the pco_ of Object(addr.=%X) is nullptr",this);
                    else if(each->pco_->handle.done()) env_err("in Env(addr.=%X), the pco_ of Object(addr.=%X) has already done",this);
                    else each->pco_->handle.resume();
                }
                corotasks_.erase(step_);
            }

            run_event:
            if(event_queue_.size()){
                if(event_queue_.top() == step_){
                    const_cast<Event&>(event_queue_.top()).trigger();
                    event_queue_.pop();
                    goto run_event;
                }
            }

            for(auto& i : objs_){
                i->tick(step_);
            }
            step_++;
        }

        for(auto& i : objs_){
            i->stop();
        }
        status_ = Status::STOPPED;
    }else{ // connot run
        warn("the Env(addr.=%X) is already running",this);
    }
}
void Env::stop(){
    if(this->status_==Status::STARTED){
        this->status_ == Status::STOPPING;
    }
    else { // cannot stop
        warn("the Env(addr.=%X) has not started yet",this);
    }
}
template<typename _Fn,typename ... _Args>
void Env::set_timeout(_Fn &&f,size_t _timeout, _Args &&...args){
    if(_timeout==0){
        warn("in function: %s (_timeout == 0) will directly launch inputted function.",__FUNCTION__);
        f(args...);
        return;
    }
    event_queue_.emplace(Event(_timeout+step_,std::forward<_Fn&&>(f),std::forward<_Args>(args)...));
    return;
}
template<typename T, typename ...Args>
requires(std::is_base_of_v<Object,T>)
T* Env::emplace_obj(Args&& ...args){
    T* res = new T(std::forward<Args&&>(args)...);
    res->env = this;
    this->objs_.emplace(res);
    return res;
}
template<typename T>
requires(std::is_base_of_v<Object,T>)
void Env::erase_obj(T* obj){
    if(this->objs_.count(obj)){
        this->objs_.erase(obj);
        obj->__release();
        delete obj;
    }else{
        warn("Attempted to erase an Object(addr.=%x typeid=%s) not belong to Env(addr.=%X)."
            ,obj,typeid(T).name(),this);
    }
}
template<Env::LogLevel level,typename ...Args> 
inline constexpr void Env::__log(const char* fmt, Args&& ...args)const{
    if constexpr (level == LogLevel::NORMAL){
        printf("LOG [%d] ",step_);
        printf(fmt, std::forward<Args>(args)...);
        printf("\n");
    }else if constexpr(level == LogLevel::DEBUG){
#if defined(_DEBUG) || defined(_DBG) ||defined(_debug) || defined(_dbg)
        printf("DEBUG [%d] ",step_);
        printf(fmt, std::forward<Args>(args)...);
        printf("\n");
#endif
    }else if constexpr(level == LogLevel::WARNING){
        printf("WAARNING [%d] ",step_);
        printf(fmt, std::forward<Args>(args)...);
        printf("\n");
    }else if constexpr(level == LogLevel::USER_ERROR){
        printf("USER_ERROR [%d] ",step_);
        printf(fmt, std::forward<Args>(args)...);
        printf("\n");
    }else if constexpr(level == LogLevel::ENV_ERROR){
        printf("ENV_ERROR [%d] ",step_);
        printf(fmt, std::forward<Args>(args)...);
        printf("\n!!! Please report this issue to https://github.com/cenxuantian."
            " Thank you very much. !!!\n");
    }else if constexpr(level == LogLevel::SYS_ERROR){
        printf("SYS_ERROR [%d] ",step_);
        printf(fmt, std::forward<Args>(args)...);
        printf("\n>>> You can contact the author of this library "
            "or visit https://github.com/cenxuantian for more details <<<\n");
    }else{
        printf("UNKNOWN [%d] PLEASE REPORT THIS TO "
        "[https://github.com/cenxuantian] "
        "BECAUSE THERE IS BUG IN THE [" __FILE__ "] \n"
        "HERE IS YOUR ORIGINAL LOG:\n",step_);
        printf(fmt, std::forward<Args>(args)...);
        printf("\n");
    }
}
template<typename ...Args> inline constexpr void Env::log(const char* fmt, Args&& ...args) const{
    __log<LogLevel::NORMAL>(fmt,std::forward<Args&&>(args)...);
}
template<typename ...Args> inline constexpr void Env::logd(const char* fmt, Args&& ...args) const{
    __log<LogLevel::DEBUG>(fmt,std::forward<Args&&>(args)...);
}
template<typename ...Args> inline constexpr void Env::warn(const char* fmt, Args&& ...args) const{
    __log<LogLevel::WARNING>(fmt,std::forward<Args&&>(args)...);
}
template<typename ...Args> inline constexpr void Env::err(const char* fmt, Args&& ...args) const{
    __log<LogLevel::USER_ERROR>(fmt,std::forward<Args&&>(args)...);
}
template<typename ...Args> inline constexpr void Env::sys_err(const char* fmt, Args&& ...args) const{
    __log<LogLevel::SYS_ERROR>(fmt,std::forward<Args&&>(args)...);
}
template<typename ...Args> inline constexpr void Env::env_err(const char* fmt, Args&& ...args) const{
    __log<LogLevel::ENV_ERROR>(fmt,std::forward<Args&&>(args)...);
}
// Object
Object::~Object(){}
void  Object::__start(){
    if(pco_){delete pco_;pco_=nullptr;}
    pco_ = new Coro(this->start());
}
void Object::__resume(){
    if(this->pco_) this->pco_->resume();
    else {
        env->env_err("Object(addr.=%x) do not has a pointer of coroutine.");
        return;
    }
}
void Object::__release(){
    if (pco_) {
        delete pco_;
        pco_=nullptr;
    }
}

}}