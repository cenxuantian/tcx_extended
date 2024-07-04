cmake_minimum_required(VERSION 3.0.0)
project(t_extended VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/tcx_standalone)
include_directories(${PROJECT_SOURCE_DIR}/tcx_net)

# main debug/development test
add_executable(t_main_test main.cpp)

# svg test
add_executable(t_svg_test examples/svg_test.cpp)

# http test
add_executable(t_http_test examples/http_test.cpp)
target_link_libraries(t_http_test ws2_32)

# very easy http get test
add_executable(http_client_test examples/http_client_test.cpp)
target_link_libraries(http_client_test ws2_32)

# http server test
add_executable(http_server_test examples/http_server_test.cpp)
target_link_libraries(http_server_test ws2_32)

# socket wrapper functions test
add_executable(t_sock_test examples/socket_test.cpp)
target_link_libraries(t_sock_test ws2_32)

# simulation module test
add_executable(sim_test examples/sim_test.cpp)