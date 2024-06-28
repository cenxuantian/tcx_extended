#pragma once
#include <thread>
#include <unordered_set>
#include <functional>
#include <chrono>
#include <memory>
#include <future>
#include <queue>
#include <coroutine>

namespace tcx{
class Object;
class Env;
class EnvRef;

struct SimSus
{
private:
    const size_t steps_;
public:
    SimSus(size_t wait_step):steps_(wait_step){}
    constexpr bool await_ready() const noexcept { return steps_==0;}
    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};

class SimCoro{
public:
    struct promise_type{
        SimCoro get_return_object(){
            return SimCoro(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        auto initial_suspend()noexcept {return std::suspend_never{};};
        auto final_suspend()noexcept {return std::suspend_never{};};
        auto yield_value()noexcept{return std::suspend_never{};}
        void return_value() noexcept{}
        void unhandled_exception(){}
    };

    std::coroutine_handle<promise_type> handle;
    explicit SimCoro(std::coroutine_handle<promise_type> const& h):handle(h){}
    ~SimCoro(){handle.destroy();}
    void resume(){handle.resume();}
};

class EnvRef{
    friend class Object;
private:
    Env& env_;
    Object* ref_by_;
public:
    EnvRef(Env& env):env_(env),ref_by_(nullptr){}
    void set_ref(Object* obj){ref_by_ = obj;}
    size_t now();
    SimSus sleep(size_t steps);
};

class Object{
    friend class Env;
protected:
    EnvRef env_;
    SimCoro* pco_=nullptr;
    void  __start(){
        if(pco_){delete pco_;pco_=nullptr;}
        pco_ = new SimCoro(this->start());
    }
public:
    Object(Env& env);
    ~Object();
    virtual SimCoro start() = 0;
    virtual void stop() = 0;
    virtual void tick(size_t step_) = 0;
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
    struct Task{
        size_t step;
        Object* target;
        bool operator<(Task const& other) const noexcept{return step<other.step;}
        bool operator==(size_t num) const noexcept{return step==num;}
    };
    Status status_ = Status::STOPPED;
    size_t step_ = 0;
    std::unordered_set<Object*> objs_;
    std::priority_queue<Task> queue_;

    size_t now(){return step_;}
public:
    void run(size_t max_step = std::string::npos){
        if(this->status_==Status::STOPPED){
            for(auto& i : objs_){
                i->__start();
            }
            status_ = Status::STARTED;

            while(status_ == Status::STARTED && (max_step == std::string::npos || step_<=max_step)){
                loop:
                if(queue_.size()){
                    if(queue_.top() == step_){
                        Object* tar = queue_.top().target;
                        queue_.pop();
                        if(tar) if(tar->pco_) tar->pco_->resume();
                        goto loop;
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
    void stop(){
        if(this->status_==Status::STARTED){
            this->status_ == Status::STOPPING;
        }
        else {
            // cannot stop
        }
    }
};

size_t EnvRef::now(){return env_.now();}
SimSus EnvRef::sleep(size_t steps){
    if(steps != 0)env_.queue_.emplace(steps+env_.step_,(Object*)ref_by_);
    return SimSus{steps};
}

Object::Object(Env& env):env_(env){
    env_.set_ref(this);
    env_.env_.objs_.emplace(this);
    
}
Object::~Object(){
    env_.env_.objs_.erase(this);
    if (pco_) {
        delete pco_;
        pco_=nullptr;
    }
}
}