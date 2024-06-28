cmake_minimum_required(VERSION 3.0.0)
project(t_extended VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/tcx_standalone)
include_directories(${PROJECT_SOURCE_DIR}/tcx_net)

add_executable(t_main_test main.cpp)
add_executable(t_svg_test examples/svg_test.cpp)
add_executable(t_http_test examples/http_test.cpp)
add_executable(t_sock_test examples/socket_test.cpp)
target_link_libraries(t_sock_test ws2_32)
add_executable(sim_test examples/sim_test.cpp)