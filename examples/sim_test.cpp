
#include <t_simulaltion.hpp>
#include <iostream>



class Box:public tcx::Object{
    int x;
    int y;
public:
    Box(tcx::Env& env):tcx::Object(env){}
    ~Box(){}
    tcx::SimCoro start(){
        x = 0;
        y = 0;
        co_await this->env_.sleep(10);
        x = 10;
        y = 10;
        co_await this->env_.sleep(10);
        x = 20;
        y = 20;
    }
    void stop(){}
    void tick(size_t step_){
        std::cout << env_.now() << " x:"<<x<<" y:"<<y<<'\n';
    }
};

int main(){
    tcx::Env env;
    Box b(env);
    env.run(30);

}