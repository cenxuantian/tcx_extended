#include <t_http.hpp>
#include <iostream>

int main(){
    auto ret = tcx::HTTP_GET("www.google.com");
    if(ret.second.has_value()){
        std::cout << ret.second.value().body.c_str() <<'\n';
    }
    return 0;
}