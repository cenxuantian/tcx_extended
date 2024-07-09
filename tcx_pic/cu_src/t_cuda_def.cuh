#ifndef __T_CUDA_DEF_CUH__
#define __T_CUDA_DEF_CUH__

#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>
#include <climits>
#include <iostream>

#define __cu_check_ret(_err, ...)\
    if(_err != cudaSuccess){\
        std::cerr << "CUDA Runtime Error at: " << __FILE__ << ":" << __LINE__\
                    << cudaGetErrorString(err) << " " << __FUNCTION__ << std::endl;\
        return __VA_ARGS__; \
    }(void)0

#define __cu_check(_err,...)\
    if(_err != cudaSuccess){\
        std::cerr << "CUDA Runtime Error at: " << __FILE__ << ":" << __LINE__\
                    << cudaGetErrorString(err) << " " << __FUNCTION__ << std::endl;\
        __VA_ARGS__; \
    }(void)0

#define __CU_MAX_BLOCK_COUNT 65535
#define __CU_MAX_THREAD_COUNT 1024

#define t_cuIdx threadIdx.x+blockIdx.x*blockDim.x
#define t_cuMaxThreadCount blockDim.x*gridDim.x

void t_cu_get_launch_arg(int max_size, int* block, int* thread) {
    if (max_size <= __CU_MAX_THREAD_COUNT) {
        *thread = max_size;
        *block = 1;
    }
    else {
        *thread = __CU_MAX_THREAD_COUNT;
        *block = std::min(
            (max_size % __CU_MAX_THREAD_COUNT ?
                max_size / __CU_MAX_THREAD_COUNT :
                max_size / __CU_MAX_THREAD_COUNT + 1),
            __CU_MAX_BLOCK_COUNT);
    }
}
#endif