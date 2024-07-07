#include "../include/t_picture.h"
#include <cuda_runtime.h>

int t_pic_where(){
    return -1;
}

int t_pic_load_pics(t_pics** pics,const char* path){
    cudaMalloc(pics,sizeof(t_pics));
    return T_PIC_OK;
}

void t_pic_release(t_pics* pics){
    cudaFree(pics);
}

int t_pic_to_gray(t_pics* pics){
    return T_PIC_OK;
}
