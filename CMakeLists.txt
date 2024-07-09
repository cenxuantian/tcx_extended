cmake_minimum_required(VERSION 3.0.0)


set(ENABLE_CUDA_FOR_T_PIC TRUE)
set(ENABLE_CUDA_FOR_T_MATRIX TRUE)

set(BUILD_MAIN_TEST FALSE)
set(BUILD_SVG_TEST FALSE)
set(BUILD_HTTP_TEST FALSE)
set(BUILD_HTTP_CLIENT_TEST FALSE)
set(BUILD_HTTP_SERVER_TEST FALSE)
set(BUILD_SOCK_TEST FALSE)
set(BUILD_SIMULATION_TEST FALSE)
set(BUILD_LOAD_DLL_TEST FALSE)
set(BUILD_PIC_TEST FALSE)
set(BUILD_MATRIX_TEST FALSE)
set(BUILD_CONTINOOUS_HEAP_TEST TRUE)


if(MINGW)
project(t_extended VERSION 0.1.0 LANGUAGES C CXX)
else()
project(t_extended VERSION 0.1.0 LANGUAGES C CXX CUDA)

FIND_PACKAGE(CUDA REQUIRED)
INCLUDE_DIRECTORIES( ${CUDA_INCLUDE_DIRS})
link_directories(${CUDA_INCLUDE_DIRS}/../lib/x64)

endif()
set(CMAKE_CXX_STANDARD 20)


include_directories(${PROJECT_SOURCE_DIR}/tcx_standalone)
include_directories(${PROJECT_SOURCE_DIR}/tcx_continuous_container)
include_directories(${PROJECT_SOURCE_DIR}/tcx_net)




# ----------------- targets -----------------

# main debug/development test
if(BUILD_MAIN_TEST)
add_executable(t_main_test main.cpp)
endif()

# svg test
if(BUILD_SVG_TEST)
add_executable(t_svg_test examples/svg_test.cpp)
endif()

# http test
if(BUILD_HTTP_TEST)
add_executable(t_http_test examples/http_test.cpp)
target_link_libraries(t_http_test ws2_32)
endif()

# very easy http get test
if(BUILD_HTTP_CLIENT_TEST)
add_executable(http_client_test examples/http_client_test.cpp)
target_link_libraries(http_client_test ws2_32)
endif()

# http server test
if(BUILD_HTTP_SERVER_TEST)
add_executable(http_server_test examples/http_server_test.cpp)
target_link_libraries(http_server_test ws2_32)
endif()

# socket wrapper functions test
if(BUILD_SOCK_TEST)
add_executable(t_sock_test examples/socket_test.cpp)
target_link_libraries(t_sock_test ws2_32)
endif()

# simulation module test
if(BUILD_SIMULATION_TEST)
add_executable(sim_test examples/sim_test.cpp)
endif()

# load cuda dll test
if(BUILD_LOAD_DLL_TEST)
add_executable(load_dll_test examples/load_dll_test.cpp)
endif()

if(BUILD_PIC_TEST)
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
endif()


if(BUILD_MATRIX_TEST)
if(MSVC)
if(ENABLE_CUDA_FOR_T_MATRIX)
add_executable(cuda_matrix_test examples/cuda_matrix_test.cpp ${PROJECT_SOURCE_DIR}/tcx_pic/cu_src/t_matrix.cu)
target_include_directories(cuda_matrix_test PUBLIC ${PROJECT_SOURCE_DIR}/tcx_pic/include)
set_target_properties(cuda_matrix_test PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set_property(TARGET cuda_matrix_test PROPERTY CUDA_ARCHITECTURES OFF)
endif()
endif()
endif()


if(BUILD_CONTINOOUS_HEAP_TEST)
add_executable(continous_heap_test 
	examples/continuous_heap_test.c
	${PROJECT_SOURCE_DIR}/tcx_continuous_container/t_continuous_heap.c)
endif()