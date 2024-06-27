#include <t_sock_wrapper.hpp>
#include <t_meta.hpp>
#include <iostream>


int main(){
    tcx::Socket sock = tcx::Socket::create(tcx::Socket::Protocol::TCP).value();
    sock.bind(tcx::IPAddr::v4("127.0.0.1",887));
    if(sock.listen()){
        std::cout << "socket listening\n";
    }

    return 0;
}