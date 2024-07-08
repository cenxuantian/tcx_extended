#include "../include/t_picture.h"
#include <cuda_runtime.h>


__global__ void g_t_pic_rgb_to_gray(t_pics* pics){
    int size = pics->height * pics->width * 3;
    char* cur_buf = pics->data + threadIdx.x * size;
    for(int i = 0 ; i < size;i+=3){
        int gray = (cur_buf[i]*19595 + cur_buf[i+1]*38469 + cur_buf[i+2]*7472) >> 16;
        cur_buf[i] = gray;
        cur_buf[i+1] = gray;
        cur_buf[i+2] = gray;
    }
}


int t_pic_where(){
    return -1;
}

int t_pic_load_pics(t_pics** pics,const char* path){
    cudaMalloc(pics,sizeof(t_pics));
    (*pics)->height = 0;
    (*pics)->width = 0;
    (*pics)->count = 1;
    (*pics)->data = NULL;
    return T_PIC_OK;
}

void t_pic_release(t_pics* pics){
    cudaFree(pics);
}

int t_pic_to_gray(t_pics* pics){
    switch(pics->color_space){
        case t_pic_rgb:{
            g_t_pic_rgb_to_gray<<<1,pics->count>>>(pics);
            break;
        }
    }
    return T_PIC_OK;
}
