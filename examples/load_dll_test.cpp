#include <windows.h>
#include <iostream>

using pic_process_rgba_t = int (*)(void* buf, int width, int height, int pixel);

int main(){

    HMODULE h = LoadLibraryA( "D:/User/Desktop/git_proj/nvcc_test/x64/Release/t_pic_processor.dll");
    if(!h) return -1;
    pic_process_rgba_t pic_process_rgba = (pic_process_rgba_t)GetProcAddress(h,"pic_process_rgba");
    if(!pic_process_rgba) return -1;
    
    char* buf = new char[1024*4];
    memset(buf,1,1024*4);
    int ret = pic_process_rgba(buf,1,1,1024);
    
	for (int i = 0; i < 1024*4; i++) {
		printf("%d ", ((char*)(buf))[i]);
	}

    delete[] buf;
    return 0;
}