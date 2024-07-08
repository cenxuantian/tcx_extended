#include "../include/t_picture.h"
#ifdef _WIN32
#include <atlimage.h>
#endif

static int __load_img(t_pics** dist,const char* path) {
#ifdef _WIN32
	BITMAP bm;
	CImage img;
	HRESULT hr = img.Load(path);
	if (!SUCCEEDED(hr)) return T_PIC_ERR;
	HBITMAP hbmp = img;
	if (!GetObject(hbmp, sizeof(bm), &bm)) return T_PIC_ERR;
	img.Destroy();
	return T_PIC_OK;
#else
	retuurn T_PIC_ERR;
#endif
}
