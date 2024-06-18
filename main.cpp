#include "tcx_standalone/t_blob.hpp"
#include "tcx_standalone/t_interval_map.hpp"
#include "tcx_standalone/t_random.hpp"
#include "tcx_standalone/t_shared.hpp"
#include <iostream>

int main(){
        tcx::DeviceMutex m("m1");
        m.lock();
        std::cout << "lock ok\n";
        system("pause");
        m.unlock();;
        std::cout << "unlock ok\n";
        system("pause");
    return 0;
}