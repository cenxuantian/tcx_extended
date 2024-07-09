#ifndef __T_CONTINUOUS_HEAP_H__
#define __T_CONTINUOUS_HEAP_H__

#ifdef DLL_EXPORT
#define T_CONTINUOUS_API __declspec(dllexport)
#elif defined(DLL_IMPORT)
#define T_CONTINUOUS_API __declspec(dllimport)
#else
#define T_CONTINUOUS_API
#endif


#ifdef __cpluspluls
extern "C"{
#endif


// ----------------------- definitions -----------------------
#define t_cont_heap_mem_smaller -1
#define t_cont_heap_mem_greater 1
#define t_cont_heap_custom_compare 0


typedef int(*t_cont_heap_cmp_fn_t)(void*, void*);

/*
* Create a continuous_heap
* [IN] each_size - sizeof the item
* [IN] comp_type - t_cont_heap_mem_smaller or t_cont_heap_mem_greater
* [IN] reserve - reserve the space for the heap, the capacity will >= the reserve
* [RET] the heap
*/
T_CONTINUOUS_API void* t_cont_heap_create(int each_size, int cmp_type, int reserve);

// ----------------------- non-const func -----------------------
/*
* if you load the data from other place e.g. shared mmemory, socket and filesyste, you need to run this function again
* [IN] heap - the heap data
* [IN] fn - the function to compare the item
*/
T_CONTINUOUS_API void t_cont_heap_set_cmp_fn(void* heap, t_cont_heap_cmp_fn_t fn);

/*
* [IN] heap - the heap data
* [IN] buf - the item that you want push into the heap
* [RET] the new heap sometimes different from the heap inputted, remember to use free
*/
T_CONTINUOUS_API void* t_cont_heap_push(void* heap,void* buf);
#define t_cont_heap_push_lv(heap,data) heap = t_cont_heap_push(heap,&data)
#define t_cont_heap_push_rv(heap,__typename,data) {__typename _ = data;heap = t_cont_heap_push(heap,&_);}(void)0
/*
* [IN] heap - the heap data
* [RET] the new heap sometimes different from the heap inputted
*/
T_CONTINUOUS_API void t_cont_heap_pop(void* heap);
/*
* [IN] heap - the heap data
* [IN] size - the target size you want to reserve
* [RET] the new heap sometimes different from the heap inputted
*/
T_CONTINUOUS_API void* t_cont_heap_reserve(void* heap, int size);

// ----------------------- const func -----------------------
/*
* [IN] heap - the heap data
* [RET] the pointer point to the start address of the top item in this heap. which is exactly the same as t_cont_heap_data returns
*/
T_CONTINUOUS_API void* t_cont_heap_top(const void* heap);
#define t_cont_heap_top_as(heap,__typename) (*((__typename*)t_cont_heap_top(heap)))
/*
* [IN] heap - the heap data
* [RET] the real data of the heap, which can be used for itereation
*/
T_CONTINUOUS_API void* t_cont_heap_data(const void* heap);

/*
* [IN] heap - the heap data
* [RET] the item size of this heap
*/
T_CONTINUOUS_API int t_cont_heap_item_size(const void* heap);

/*
* [IN] heap - the heap data
* [RET] the capacity of this heap
*/
T_CONTINUOUS_API int t_cont_heap_capacity(const void* heap);

/*
* [IN] heap - the heap data
* [RET] 1 if tempty, 0 if not empty
*/
T_CONTINUOUS_API int t_cont_heap_empty(const void* heap);

/*
* [IN] heap - the heap data
* [RET] the size
*/
T_CONTINUOUS_API int t_cont_heap_size(const void* heap);

/*
* [IN] heap - the heap data
* [RET] the capacity of this heap
*/
T_CONTINUOUS_API int t_cont_heap_capacity(const void* heap);

/*
* [IN] heap - the heap data
* [RET] the total size of the buf
*/
T_CONTINUOUS_API int t_cont_heap_total_size(const void* heap);



// ----------------------- compare functions -----------------------
T_CONTINUOUS_API int t_cont_heap_item_cmp_i64(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_u64(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_i32(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_u32(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_i16(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_u16(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_i8(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_u8(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_f32(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_f64(void* _1, void* _2);

T_CONTINUOUS_API int t_cont_heap_item_cmp_i64_r(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_u64_r(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_i32_r(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_u32_r(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_i16_r(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_u16_r(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_i8_r(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_u8_r(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_f32_r(void* _1, void* _2);
T_CONTINUOUS_API int t_cont_heap_item_cmp_f64_r(void* _1, void* _2);
#ifdef __cpluspluls
}
#endif
#endif