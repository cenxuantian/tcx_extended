cmake_minimum_required(VERSION 3.0.0)
project(t_extended VERSION 0.1.0 LANGUAGES C CXX)

include_directories(${PROJECT_SOURCE_DIR}/tcx_standalone)


add_executable(t_main_test main.cpp)
add_executable(t_svg_test examples/svg_test.cpp)
