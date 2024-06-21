#include "tcx_standalone/t_blob.hpp"
#include "tcx_standalone/t_interval_map.hpp"
#include "tcx_standalone/t_random.hpp"
#include "tcx_standalone/t_shared.hpp"
#include "tcx_standalone/t_trie.hpp"
#include "tcx_standalone/t_file.hpp"
#include <iostream>

int main(){

    tcx::Path a = tcx::Path::cwd();
    a.cd("..").mkdir("this is a test dir");
    return 0;
}