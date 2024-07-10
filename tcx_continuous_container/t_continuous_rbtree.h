#ifndef __T_CONTINUOUS_RBTREE_H__
#define __T_CONTINUOUS_RBTREE_H__


#ifdef DLL_EXPORT
#define T_CONTINUOUS_API __declspec(dllexport)
#elif defined(DLL_IMPORT)
#define T_CONTINUOUS_API __declspec(dllimport)
#else
#define T_CONTINUOUS_API
#endif


#ifdef __cpluspluls
extern "C" {
#endif


#define t_cont_rbtree_mem_smaller -1
#define t_cont_rbtree_mem_greater 1
#define t_cont_rbtree_custom_compare 0



typedef int(*t_cont_rbtree_cmp_fn_t)(void*, void*);


T_CONTINUOUS_API void* t_cont_rbtree_create(int each_size, int cmp_type, int reserve);
T_CONTINUOUS_API void t_cont_rbtree_set_cmp_fn(void* rbtree, t_cont_rbtree_cmp_fn_t fn);

T_CONTINUOUS_API void* t_cont_rbtree_emplace(void* rbtree, void* buf);

T_CONTINUOUS_API int t_cont_rbtree_seaarch(void* rbtree, void* data);
T_CONTINUOUS_API void* t_cont_rbtree_at(void* rbtree, int pos);
T_CONTINUOUS_API void t_cont_rbtree_erase(void* rbtree, int pos);


T_CONTINUOUS_API void* t_cont_rbtree_reserve(void* rbtree, int size);
T_CONTINUOUS_API void* t_cont_rbtree_data(const void* rbtree);
T_CONTINUOUS_API int t_cont_rbtree_item_size(const void* rbtree);
/*
* [IN] rbtree - the rbtree data
* [RET] the capacity of this rbtree
*/
T_CONTINUOUS_API int t_cont_rbtree_capacity(const void* rbtree);
T_CONTINUOUS_API int t_cont_rbtree_empty(const void* rbtree);
/*
* [IN] rbtree - the rbtree data
* [RET] the size
*/
T_CONTINUOUS_API int t_cont_rbtree_size(const void* rbtree);

/*
* [IN] rbtree - the rbtree data
* [RET] the capacity of this rbtree
*/
T_CONTINUOUS_API int t_cont_rbtree_capacity(const void* rbtree);

/*
* [IN] rbtree - the rbtree data
* [RET] the total size of the buf
*/
T_CONTINUOUS_API int t_cont_rbtree_total_size(const void* rbtree);






#ifdef __cpluspluls
}
#endif
#endif