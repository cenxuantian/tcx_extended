#include <iostream>

#include <t_picture.h>

int main(){
    printf("hello from cuda_pic_test!\n");
    if(t_pic_where() > 0) {
        printf("cpu process your pic\n");
        return 0;
    }else{
        printf("gpu process your pic\n");
    }

    t_pic pic;
    t_pic_load_pics(&pic, "D:\\User\\Desktop\\glfwtest\\img_mars.jpg");
    t_pic_to_gray(&pic);
    t_pic_save(&pic, "D:\\User\\Desktop\\img_mars.jpg");
    t_pic_release(&pic);

    return 0;
}