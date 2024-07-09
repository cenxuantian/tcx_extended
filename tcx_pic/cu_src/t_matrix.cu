#include "../include/t_matrix.h"

#include "t_cuda_def.cuh"



int t_mat_create(t_mat * mat) {
    int total_size = mat->width * mat->height * mat->each_size;
    auto err = cudaMalloc(&mat->data, total_size);
    __cu_check_ret(err, T_MAT_ERR);
    err = cudaMemset(mat->data, 0, total_size);
    __cu_check_ret(err, T_MAT_ERR);
    return T_MAT_OK;
}
void t_mat_release(t_mat* mat) {
    if (mat->data) {
        cudaFree(mat->data);
        mat->data = NULL;
    }
    mat->height = 0;
    mat->width = 0;
}

// add
__global__ void g_matmat_add_int(void* __restrict__ _1, void* __restrict__ _2, int width) {
    int* _a = (int*)_1;
    int* _b = (int*)_2;

    int cur_row = blockIdx.x;
    int cur_cow = threadIdx.x;

    loop:
    _a[cur_row * width + cur_cow] += _b[cur_row * width + cur_cow];

    cur_cow += blockDim.x;
    if (cur_cow<width) {
        goto loop;
    }
    return;

}
__global__ void g_matmat_add_double(void* __restrict__ _1, void* __restrict__ _2, int width) {
    double* _a = (double*)_1;
    double* _b = (double*)_2;

    int cur_row = blockIdx.x;
    int cur_cow = threadIdx.x;

loop:
    _a[cur_row * width + cur_cow] += _b[cur_row * width + cur_cow];

    cur_cow += blockDim.x;
    if (cur_cow < width) {
        goto loop;
    }
    return;

}
__global__ void g_matnum_multi_int(void* _1, double num, int width) {
    int* _a = (int*)_1;

    int cur_row = blockIdx.x;
    int cur_cow = threadIdx.x;

loop:
    _a[cur_row * width + cur_cow] *= num;

    cur_cow += blockDim.x;
    if (cur_cow < width) {
        goto loop;
    }
    return;
}
__global__ void g_matnum_multi_double(void* _1, double num, int width) {

    double* _a = (double*)_1;

    int cur_row = blockIdx.x;
    int cur_cow = threadIdx.x;

loop:
    _a[cur_row * width + cur_cow] *= num;

    cur_cow += blockDim.x;
    if (cur_cow < width) {
        goto loop;
    }
    return;
}
__global__ void g_mat_add_num_int(void* _1, double num, int max_idx) {
    int i = t_cuIdx;
    int max_thread = t_cuMaxThreadCount;
    int* data = (int*)_1;
    
    while (i < max_idx) {
        data[i] += num;
        i += max_thread;
    }
}
__global__ void g_mat_add_num_double(void* _1, double num, int max_idx) {
    int i = t_cuIdx;
    int max_thread = t_cuMaxThreadCount;
    double* data = (double*)_1;

    while (i < max_idx) {
        data[i] += num;
        i += max_thread;
    }
}
int t_mat_add_mat(t_mat* _1, t_mat* _2) {
    if (!(_1->width == _2->width && _1->height == _2->height && _1->each_size == _2->each_size)) {
        return T_MAT_ERR;
    }
    if (_1->each_size == sizeof(int)) {
        g_matmat_add_int << < _1->height, min(_1->width, 1024) >> > (_1->data, _2->data, _1->width);
    }
    else if (_1->each_size == sizeof(double)) {
        g_matmat_add_double << < _1->height, min(_1->width, 1024) >> > (_1->data, _2->data, _1->width);
    }
    return T_MAT_OK;
}
int t_mat_add_num(t_mat* _1, double num) {
    int max_idx = _1->height * _1->width;
    int block, thread;
    t_cu_get_launch_arg(max_idx, &block, &thread);
    if (_1->each_size == sizeof(int)) {
        g_mat_add_num_int << <block, thread >>> (_1->data, num, max_idx);
    }
    else if (_1->each_size == sizeof(double)) {
        g_mat_add_num_double << <block, thread >>> (_1->data, num, max_idx);
    }
    
    return T_MAT_OK;
}
int t_mat_multiply_num(t_mat* _1, double num) {
    if (_1->each_size == sizeof(int)) {
        g_matnum_multi_int << <_1->height, min(_1->width, 1024) >> > (_1->data, num, _1->width);
    }
    else if (_1->each_size == sizeof(double)) {
        g_matnum_multi_double << <_1->height, min(_1->width, 1024) >> > (_1->data, num, _1->width);
    }
    return T_MAT_OK;
}


int t_mat_transpos(t_mat* mat) {
    return T_MAT_OK;
}
int t_mat_clone(t_mat* dist, t_mat* src) {
    dist->each_size = src->each_size;
    dist->height = src->each_size;
    dist->width = src->width;
    int ret = t_mat_create(dist);
    if (ret == T_MAT_ERR) return ret;
    auto err = cudaMemcpy(dist->data, src->data, dist->height * dist->width * dist->each_size, cudaMemcpyDeviceToDevice);
    __cu_check(err, T_MAT_ERR);
    return T_MAT_OK;
}
int t_mat_inverse(t_mat* mat) {
    return T_MAT_OK;
}

int t_mat_get_int(t_mat* mat,int x, int y) {
    int ret;
    t_mat_get(mat, x, y, &ret, sizeof(ret));
    return ret;
}
double t_mat_get_double(t_mat* mat,int x, int y) {
    double ret;
    t_mat_get(mat, x, y, &ret, sizeof(ret));
    return ret;
}
int t_mat_get(t_mat* mat,int x, int y, void* out, int size_of_out) {
    auto err = cudaMemcpy(out, (char*)(mat->data) + (y * mat->width + x * size_of_out), size_of_out, cudaMemcpyDeviceToHost);
    __cu_check(err, T_MAT_ERR);
    return T_MAT_OK;
}
int t_mat_set_int(t_mat* mat,int x, int y, int val) {
    return t_mat_set(mat, x, y, &val, sizeof(val));
}
int t_mat_set_double(t_mat* mat,int x, int y, double val) {
    return t_mat_set(mat, x, y, &val, sizeof(val));
}
int t_mat_set(t_mat* mat,int x, int y, void* in, int size_of_in) {
    auto err = cudaMemcpy((char*)(mat->data) + ((y * mat->width + x) * size_of_in), in, size_of_in, cudaMemcpyHostToDevice);
    __cu_check(err, T_MAT_ERR);
    return T_MAT_OK;
}

__global__ void t_mat_print_int(int* data, int width,int height) {
    for (int i = 0; i < height;i++) {
        for (int j = 0; j < width; j++) {
            printf("%d ", data[i * width + j]);
        }
        printf("\n");
    }
}
__global__ void t_mat_print_double(double* data, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%d ", data[i * width + j]);
        }
        printf("\n");
    }
}
void t_mat_print(t_mat* mat) {
    if (mat->each_size == sizeof(int)) {
        t_mat_print_int<<<1,1>>>((int*)mat->data, mat->width, mat->height);
    }
    else if (mat->each_size == sizeof(double)) {
        t_mat_print_double<<<1,1>>>((double*)mat->data, mat->width, mat->height);
    }
}
void t_mat_synchronize() {
    cudaDeviceSynchronize();
    auto err = cudaGetLastError();
    __cu_check(err);
}









