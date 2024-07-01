
#include <t_simulaltion.hpp>
#include <iostream>
#include <math.h>
#include <map>


using namespace tcx;
class Car: public sim::Object{
public:
    double pos = 0;
    double speed = 0;
    double acceleration = 0;
    bool forward = true;
    Car(){}
    void move(){
        speed+=acceleration;
        if(speed < 0) {
            speed = 0;
            acceleration = 0;
        }
        double dist = speed*1;
        if(forward) pos+=dist;
        else pos-=dist;
    }
    sim::Coro start(){
        env->log("%X start on pos: %d forward: %d",this,pos,forward);
        co_await env->sleep(this,3);
        acceleration = 3;
        env->log("%X start drive",this);
        co_await env->sleep(this,5);
        acceleration = 0;
        env->log("%X reach highest speed",this);
        co_await env->sleep(this,10);
        acceleration = -1;
        env->log("%X slowing down",this);
        co_await env->sleep(this,10);
        env->log("%X keep same speed",this);
        acceleration = 0;
    }
    void stop(){}
    void tick(size_t step_){
        move();
        env->log("%X at pos:%0.f",this,pos);
    }
};

class Collision:public sim::Object{
public:
    Car* cars[2];
    sim::Coro start(){co_await std::suspend_never{};};
    void stop(){};
    void tick(size_t step){
        if(std::abs(cars[0]->pos - cars[1]->pos) <=20){
            env->log("collided");
            cars[0]->speed = 0;
            cars[1]->speed = 0;
        }
    }
};



int main(){
    sim::Env env;

    Car* car1= env.emplace_obj<Car>();
    Car* car2= env.emplace_obj<Car>();
    Collision* coll = env.emplace_obj<Collision>();


    coll->cars[0]=car1;
    coll->cars[1]=car2;

    car1->pos = 0;
    car1->forward = true;

    car2->pos = 1000;
    car2->forward = false;


    env.run(80);
}