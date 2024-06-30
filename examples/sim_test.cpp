
#include <t_simulaltion.hpp>
#include <iostream>



using namespace tcx;

class Box:public sim::Object{
    int x;
    int y;
public:
    Box(sim::Env& env):sim::Object(env){}
    ~Box(){}
    sim::Coro start(){
        x = 0;
        y = 0;

        envref_.set_timeout([&](){
            std::cout << envref_.now() <<'\n';
        },15);
        co_await envref_.sleep(10);
        x = 10;
        y = 10;
        co_await envref_.sleep(10);
        x = 20;
        y = 20;
    }
    void stop(){}
    void tick(size_t step_){
        std::cout << envref_.now() << " x:"<<x<<" y:"<<y<<'\n';
    }
};

int main(){
    sim::Env env;
    Box b(env);
    env.run(30);

}