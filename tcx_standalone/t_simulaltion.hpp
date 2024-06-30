#pragma once
#include <thread>
#include <unordered_set>
#include <functional>
#include <chrono>
#include <memory>
#include <future>
#include <queue>
#include <coroutine>
#include <any>

namespace tcx{
namespace sim{

struct Sus;
class Coro;
class Object;
class EnvRef;
class Env;
class Evnet;

struct Sus{
private:
    const size_t steps_;
public:
    Sus(size_t wait_step):steps_(wait_step){}
    constexpr bool await_ready() const noexcept {return steps_==0;}
    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};

class Coro{
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

class EnvRef{
    friend class Object;
private:
    Env& env_;
    Object* ref_by_;
    void set_ref(Object* obj);
    EnvRef(Env& env);
public:
    size_t now();
    Sus sleep(size_t steps);
    template<typename _Fn,typename ... _Args>
    void set_timeout(_Fn &&f,size_t _time_out, _Args &&...args);
    std::unordered_set<Object*>& obj_list();
};

class Object{
    friend class Env;
private:
    // this function will call the start()
    void  __start();
protected:
    // the reference of emvironments
    EnvRef envref_; 
    // the coroutine of running start() function
    Coro* pco_=nullptr;
public:
    Object(Env& env);
    ~Object();
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


class Env{
    friend class Object;
    friend class EnvRef;
private:
    enum class Status{
        STOPPED = 0,
        STARTED = 1,
        STOPPING = 2,
    };
    struct CoroTask{
        size_t step;
        Object* target;
        bool operator<(CoroTask const& other)const noexcept;
        bool operator==(size_t num)const noexcept;
    };
    Status status_ = Status::STOPPED;
    size_t step_ = 0;
    std::unordered_set<Object*> objs_;
    std::priority_queue<CoroTask> corotask_queue_;
    std::priority_queue<Event> event_queue_;
    size_t __now() const noexcept;
public:
    void run(size_t max_step = std::string::npos);
    void stop();
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
    return step_<other.step_;
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
size_t Env::__now() const noexcept{
    return step_;
}
bool Env::CoroTask::operator<(CoroTask const& other)const noexcept{
    return step<other.step;
}
bool Env::CoroTask::operator==(size_t num)const noexcept{
    return step==num;
}
void Env::run(size_t max_step){
    if(this->status_==Status::STOPPED){
        for(auto& i : objs_){
            i->__start();
        }
        status_ = Status::STARTED;

        while(status_ == Status::STARTED && (max_step == std::string::npos || step_<=max_step)){
            run_coro:
            if(corotask_queue_.size()){
                if(corotask_queue_.top() == step_){
                    Object* tar = corotask_queue_.top().target;
                    corotask_queue_.pop();
                    if(tar) if(tar->pco_) tar->pco_->resume();
                    goto run_coro;
                }
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
    }else{
        // connot run
    }
}
void Env::stop(){
    if(this->status_==Status::STARTED){
        this->status_ == Status::STOPPING;
    }
    else {
        // cannot stop
    }
}

// EnvRef
EnvRef::EnvRef(Env& env):env_(env),ref_by_(nullptr){
    // no operations
}
size_t EnvRef::now(){
    return env_.__now();
}
Sus EnvRef::sleep(size_t steps){
    if(steps != 0)env_.corotask_queue_.emplace(steps+env_.step_,(Object*)ref_by_);
    return Sus{steps};
}
void EnvRef::set_ref(Object* obj){
    ref_by_ = obj;
}
template<typename _Fn,typename ... _Args>
void EnvRef::set_timeout(_Fn &&f,size_t _timeout, _Args &&...args){
    if(_timeout==0){
        f(args...);
        return;
    }
    env_.event_queue_.emplace(Event(_timeout+env_.__now(),std::forward<_Fn&&>(f),std::forward<_Args>(args)...));
    return;
}
std::unordered_set<Object*>& EnvRef::obj_list(){return env_.objs_;}

// Object
Object::Object(Env& env):envref_(env){
    envref_.set_ref(this);
    envref_.env_.objs_.emplace(this);
}
Object::~Object(){
    envref_.env_.objs_.erase(this);
    if (pco_) {
        delete pco_;
        pco_=nullptr;
    }
}
void  Object::__start(){
    if(pco_){delete pco_;pco_=nullptr;}
    pco_ = new Coro(this->start());
}


}}