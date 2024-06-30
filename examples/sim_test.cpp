
#include <t_simulaltion.hpp>
#include <iostream>
#include <math.h>
#include <map>


using namespace tcx;

class Car: public sim::Object{
public:
    double pos_x;
    double pos_y;
    double speed;
    double acceleration;
    double towards;
    Car(){
        pos_x = 0;
        pos_y = 0;
        towards = M_PI / 4;
        acceleration=0;
        speed = 0;
    }
    void move(){
        speed+=acceleration;
        if(speed < 0) {
            speed = 0;
            acceleration = 0;
        }
        double dist = speed*1;
        pos_x+=dist / std::cos(towards);
        pos_y+=dist / std::sin(towards);
    }
    sim::Coro start(){
        co_await env->sleep(this,3);
        std::cout << this << " 1\n";
        acceleration = 3;
        co_await env->sleep(this,5);
        std::cout << this << " 2\n";
        acceleration = 0;
        co_await env->sleep(this,10);
        std::cout << this << " 3\n";
        acceleration = -1;
        co_await env->sleep(this,10);
        std::cout << this << " 4\n";
        acceleration = 0;
    }
    void stop(){}
    void tick(size_t step_){
        move();
        // std::cout << env->now() << " x:"<<pos_x<<" y:"<<pos_y<<'\n';
    }
};

class Collision:public sim::Object{
public:
    sim::Coro start(){co_await std::suspend_never{};};
    void stop(){};
    void tick(size_t step){
        double last_x;
        double last_y;
        bool first = true;
        bool get = false;
        for(auto & i:env->obj_list()){
            Car* each_car = (Car*)i;
            if(first){
                first = false;
                last_x = each_car->pos_x;
                last_y = each_car->pos_y;
                continue;
            }
            if(std::abs(each_car->pos_x - last_x) <=10 && std::abs(each_car->pos_y - last_y) <=10){
                std::cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";
                get = true;
                break;
            }else{
                last_x = each_car->pos_x;
                last_y = each_car->pos_y;
            }
        }
        for(auto & i :env->obj_list()){
            Car* each_car = (Car*)i;
            each_car->speed = 0;
        }
        
    }
};



int main(){
    sim::Env env;

    Car* car1= env.emplace_obj<Car>();
    Car* car2= env.emplace_obj<Car>();
    Collision* coll = env.emplace_obj<Collision>();

    car1->towards = 0.5 * M_PI;
    car1->pos_x = 1;
    car1->pos_y = 1;

    car2->towards = 1.5 * M_PI;
    car2->pos_x = 999;
    car2->pos_y = 999;

    env.run(50);
}