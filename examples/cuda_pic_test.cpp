#include <iostream>

#include <t_picture.h>

int main(){
    printf("hello from cuda_pic_test!\n");
    if(t_pic_where() > 0) {
        printf("cpu process your pic\n");
    }else{
        printf("gpu process your pic\n");
    }

    return 0;
}