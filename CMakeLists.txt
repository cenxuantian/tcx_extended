cmake_minimum_required(VERSION 3.0.0)


set(ENABLE_CUDA_FOR_T_PIC TRUE)

if(MINGW)
project(t_extended VERSION 0.1.0 LANGUAGES C CXX)
else()
project(t_extended VERSION 0.1.0 LANGUAGES C CXX CUDA)
endif()
set(CMAKE_CXX_STANDARD 20)

# add_library(particles STATIC
#   randomize.cpp
#   randomize.h
#   particle.cu
#   particle.h
#   v3.cu
#   v3.h
#   )
# target_compile_features(particles PUBLIC cxx_std_20)
# set_target_properties( particles PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

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


# load cuda dll test
add_executable(load_dll_test examples/load_dll_test.cpp)


if(MSVC)
if(ENABLE_CUDA_FOR_T_PIC)
aux_source_directory(${PROJECT_SOURCE_DIR}/tcx_pic/cu_src SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/tcx_pic/common SRC)
add_executable(cuda_pic_test examples/cuda_pic_test.cpp ${SRC})
target_include_directories(cuda_pic_test PUBLIC ${PROJECT_SOURCE_DIR}/tcx_pic/include)
set_target_properties(cuda_pic_test PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set_property(TARGET cuda_pic_test PROPERTY CUDA_ARCHITECTURES OFF)
else()
aux_source_directory(${PROJECT_SOURCE_DIR}/tcx_pic/cpp_src SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/tcx_pic/common SRC)
add_executable(cuda_pic_test examples/cuda_pic_test.cpp ${SRC})
target_include_directories(cuda_pic_test PUBLIC ${PROJECT_SOURCE_DIR}/tcx_pic/include)
endif()

elseif(MINGW)

aux_source_directory(${PROJECT_SOURCE_DIR}/tcx_pic/cpp_src SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/tcx_pic/common SRC)
add_executable(cuda_pic_test examples/cuda_pic_test.cpp ${SRC})
target_include_directories(cuda_pic_test PUBLIC ${PROJECT_SOURCE_DIR}/tcx_pic/include)

endif()