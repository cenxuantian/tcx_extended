#include "../include/t_picture.h"

#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>


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

__global__ static void g_t_pic_rgb_to_gray(t_pics* pics){
    int size = pics->height * pics->width * 3;
    char* cur_buf = pics->data + threadIdx.x * size;
    for(int i = 0 ; i < size;i+=3){
        int gray = (cur_buf[i]*19595 + cur_buf[i+1]*38469 + cur_buf[i+2]*7472) >> 16;
        cur_buf[i] = gray;
        cur_buf[i+1] = gray;
        cur_buf[i+2] = gray;
    }
}

__global__ static void g_t_pic_rgba_to_gray(t_pics* pics) {
    int size = pics->height * pics->width * 4;
    char* cur_buf = pics->data + threadIdx.x * size;
    for (int i = 0; i < size; i += 4) {
        int gray = (cur_buf[i] * 19595 + cur_buf[i + 1] * 38469 + cur_buf[i + 2] * 7472) >> 16;
        cur_buf[i] = gray;
        cur_buf[i + 1] = gray;
        cur_buf[i + 2] = gray;
    }
}


int t_pic_where(){
    return -1;
}

int t_pic_load_pics(t_pics** pics,const char* path){
    cudaError_t err = cudaMalloc(pics,sizeof(t_pics));
    __cu_check_ret(err,T_PIC_ERR);

    (*pics)->height = 0;
    (*pics)->width = 0;
    (*pics)->count = 1;
    (*pics)->data = nullptr;

    return T_PIC_OK;
}

void t_pic_release(t_pics* pics){
    if (!pics)return;
    if(pics->data){
        cudaError_t err = cudaFree(pics->data);
        __cu_check(err);// free error
    }
    cudaError_t err = cudaFree(pics);
    __cu_check_ret(err);
}

int t_pic_to_gray(t_pics* pics){
    switch(pics->color_space){
        case t_pic_rgb:{
            g_t_pic_rgb_to_gray<<<1,pics->count>>>(pics);
            break;
        }
        case t_pic_hsl: {
            break;
        }
        case t_pic_hsv: {
            break;
        }
        case t_pic_rgba: {
            g_t_pic_rgba_to_gray <<<1, pics->count >>> (pics);
            break;
        }
    }
    

    cudaDeviceSynchronize();
    auto err = cudaGetLastError();
    __cu_check_ret(err, T_PIC_ERR);

    return T_PIC_OK;
}
