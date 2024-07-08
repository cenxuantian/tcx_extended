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

__global__ static void g_t_pic_rgb_to_gray(char* buf, int line_byte_size){
    char* cur_buf = buf + threadIdx.x * line_byte_size;
    for(int i = 0 ; i < line_byte_size;i+=3){
        int gray = (cur_buf[i]*19595 + cur_buf[i+1]*38469 + cur_buf[i+2]*7472) >> 16;
        cur_buf[i] = gray;
        cur_buf[i+1] = gray;
        cur_buf[i+2] = gray;
    }
}

__global__ static void g_t_pic_bgr_to_gray(char* buf, int line_byte_size) {
    char* cur_buf = buf + threadIdx.x * line_byte_size;
    for (int i = 0; i < line_byte_size; i += 3) {
        int gray = (cur_buf[i+2] * 19595 + cur_buf[i + 1] * 38469 + cur_buf[i] * 7472) >> 16;
        cur_buf[i] = gray;
        cur_buf[i + 1] = gray;
        cur_buf[i + 2] = gray;
    }
}

__global__ static void g_t_pic_rgba_to_gray(char* buf, int line_byte_size) {
    char* cur_buf = buf + threadIdx.x * line_byte_size;
    for (int i = 0; i < line_byte_size; i += 4) {
        int gray = (cur_buf[i] * 19595 + cur_buf[i + 1] * 38469 + cur_buf[i + 2] * 7472) >> 16;
        cur_buf[i] = gray;
        cur_buf[i + 1] = gray;
        cur_buf[i + 2] = gray;
    }
}


int t_pic_where(){
    return -1;
}

int t_pic_load_pics(t_pic* pic, const char* path){
    if (!pic) return T_PIC_ERR;
    if (!path) return T_PIC_ERR;

    cudaError_t err;
    BITMAP bm;
    pic->img = new CImage();
    CImage& img = *pic->img;
    HRESULT hr = img.Load(path);


    
    if (!SUCCEEDED(hr)) return T_PIC_ERR;
    HBITMAP hbmp = img;
    if (!GetObject(hbmp, sizeof(bm), &bm)) {
        img.Destroy();
        return T_PIC_ERR;
    }

    void* bt1 = bm.bmBits;
    void* bt2 = img.GetBits();
    
    
    int each_pixel_size = bm.bmWidthBytes / bm.bmWidth;
    int total_size = bm.bmWidthBytes * bm.bmHeight;

    err = cudaMalloc( & pic->data, total_size);
    __cu_check_ret(err,T_PIC_ERR);

    err = cudaMemcpy(pic->data, bm.bmBits, total_size, cudaMemcpyHostToDevice);
    __cu_check_ret(err,T_PIC_ERR);


    pic->height = bm.bmHeight;
    pic->width = bm.bmWidth;
    pic->color_space = t_pic_bgr;

    return T_PIC_OK;
}

void t_pic_release(t_pic* pic){
    if (!pic)return;
    if(pic->data){
        cudaError_t err = cudaFree(pic->data);
        __cu_check(err);// free error
        pic->data = nullptr;
    }
    if (pic->img) {
        pic->img->Destroy();
        delete pic->img;
        pic->img = nullptr;
    }
}

int t_pic_to_gray(t_pic* pic){
    
    switch(pic->color_space){
        int line_byte_size;
        case t_pic_rgb:{
            line_byte_size = 3 * pic->width;
            g_t_pic_rgb_to_gray<<<1,pic->height>>>(pic->data, line_byte_size);
            break;
        }
        case t_pic_bgr: {
            line_byte_size = 3 * pic->width;
            g_t_pic_bgr_to_gray <<<1, pic->height >>> (pic->data, line_byte_size);
            break;
        }
        case t_pic_hsl: {
            line_byte_size = 3 * pic->width;
            break;
        }
        case t_pic_hsv: {
            line_byte_size = 3 * pic->width;
            break;
        }
        case t_pic_rgba: {
            line_byte_size = 4* pic->width;
            g_t_pic_rgba_to_gray << <1, pic->height >> > (pic->data, line_byte_size);
            break;
        }
    }
    

    cudaDeviceSynchronize();
    auto err = cudaGetLastError();
    __cu_check_ret(err, T_PIC_ERR);

    return T_PIC_OK;
}


int t_pic_save(t_pic* pic, const char* path) {
    if (!pic) return T_PIC_ERR;
    if (!path) return T_PIC_ERR;

    CImage& bm = *pic->img;
    int each_size = pic->color_space == t_pic_rgba? 4:3;
    int total_size = pic->width* pic->height* each_size;
    char* dist = (char*)malloc(total_size);
    cudaError_t err = cudaMemcpy(dist, pic->data,total_size, cudaMemcpyDeviceToHost);
    
    __cu_check(err, T_PIC_ERR);

    free(dist);

    HRESULT hr = bm.Save(path);
    if (!SUCCEEDED(hr)) return T_PIC_ERR;


    return T_PIC_OK;
}