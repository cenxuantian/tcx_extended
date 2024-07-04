#include <t_http.hpp>


int main(){

    tcx::HTTPServer server(tcx::IPAddr::url("0.0.0.0:5500"));
    server.listen();

    return 0;
}