#ifndef __T_MATRIX_H__
#define __T_MATRIX_H__


#ifdef DLL_EXPORT
#define T_MAT_API __declspec(dllexport)
#elif defined(DLL_IMPORT)
#define T_MAT_API __declspec(dllimport)
#else
#define T_MAT_API 
#endif

#define T_MAT_OK 0
#define T_MAT_ERR -1



#ifdef __cplusplus
extern "C" {
#endif

	struct t_mat {
		void* data;
		int width;
		int height;
		int each_size;
	};



	int t_mat_create(t_mat* mat);
	int t_mat_get_int(int x, int y);
	double t_mat_get_double(int x, int y);
	int t_mat_get(t_mat* mat,int x, int y, void* out, int size_of_out);
	int t_mat_set_int(t_mat* mat,int x, int y, int val);
	int t_mat_set_double(t_mat* mat,int x, int y, double val);
	int t_mat_set(t_mat* mat,int x, int y, void* in, int size_of_in);
	void t_mat_release(t_mat* mat);
	int t_mat_add_mat(t_mat* _1, t_mat* _2);
	int t_mat_add_num(t_mat* _1, double num);
	int t_mat_transpos(t_mat* mat);
	int t_mat_clone(t_mat* dist, t_mat* src);
	int t_mat_inverse(t_mat* mat);
	int t_mat_multiply_num(t_mat* dist, double _num);
	void t_mat_print(t_mat* mat);
	void t_mat_synchronize();


#ifdef __cplusplus
}
#endif

#endif