#ifndef __T_PICTURE_H__
#define __T_PICTURE_H__


#ifdef DLL_EXPORT
#define T_PIC_API __declspec(dllexport)
#elif defined(DLL_IMPORT)
#define T_PIC_API __declspec(dllimport)
#else
#define T_PIC_API 
#endif

#ifdef _WIN32
#include <atlimage.h>
#endif

#define T_PIC_OK 0
#define T_PIC_ERR -1

#ifdef __cplusplus
extern "C" {
#endif
    enum t_pic_color_space {
        t_pic_unknown = 0,
        t_pic_rgb = 1,
        t_pic_rgba = 2,
        t_pic_hsl = 3,
        t_pic_hsv = 4,
        t_pic_bgr = 5,
    };

    struct t_pic {
        int height;
        int width;
        char* data;
        t_pic_color_space color_space;
        CImage* img;
    };


    // tell where your pics will be processed in 
    // gpu returns -1
    // cpu returns 1
    T_PIC_API int t_pic_where();
    T_PIC_API int t_pic_load_pics(t_pic* pic, const char* path);
    T_PIC_API void t_pic_release(t_pic* pic);
    T_PIC_API int t_pic_to_gray(t_pic* pic);
    T_PIC_API int t_pic_save(t_pic* pic, const char* path);

#ifdef __cplusplus
}
#endif
#endif