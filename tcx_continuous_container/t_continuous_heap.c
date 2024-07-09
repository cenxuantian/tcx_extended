#ifndef __T_CONTINUOUS_HEAP_C__
#define __T_CONTINUOUS_HEAP_C__

#include "t_continuous_heap.h"
#include <stdlib.h>
#include <memory.h>

typedef struct {
	int each_size;
	int capacity;
	int size;
	int cmp_type;
	t_cont_heap_cmp_fn_t cmp_fn;
} continuous_heap_desc;

#define as_heap_desc(data)  ((continuous_heap_desc*)data)
#define as_byte(data) ((unsigned char*)data)

#define tree_lcidx(idx) ((idx<<1) + 1)
#define tree_rcidx(idx) ((idx<<1) + 2)
#define tree_is_rc(idx) (!(idx%2))
#define tree_is_lc(idx)	(idx%2)
#define tree_parent(idx) ((idx-1)>>1)
#define tree_is_root(idx) (idx==0)

#define heap_next_capacity(cur_capacity) (((cur_capacity + 1) << 1) - 1)
#define heap_push_pos(data) (as_heap_desc(data)->size)
#define heap_last_pos(data) (as_heap_desc(data)->size -1)
#define heap_get_data(heap) (as_byte(heap) + sizeof(continuous_heap_desc))
#define heap_at(heap,idx) (as_byte(heap_get_data(heap))+(idx*as_heap_desc(heap)->each_size))
#define heap_set(heap,idx,dataptr) memcpy(heap_at(heap, idx), dataptr, as_heap_desc(heap)->each_size)
#define heap_item_move(heap,from,to) memmove(heap_at(heap,to), heap_at(heap, from), as_heap_desc(heap)->each_size)

void* __t_cont_heap_expand(continuous_heap_desc* heap) {
	heap->capacity = heap_next_capacity(heap->capacity);
	return realloc(as_byte(heap), (heap->capacity * heap->each_size) + sizeof(continuous_heap_desc));
}

int __t_cont_heap_cmp_item(continuous_heap_desc* heap, void* _1, void* _2) {
	if (heap->cmp_type == t_cont_heap_mem_smaller) {
		return memcmp(_1, _2, heap->each_size);
	}
	else if (heap->cmp_type == t_cont_heap_mem_greater) {
		return - memcmp(_1, _2, heap->each_size);
	}
	else if (heap->cmp_type == t_cont_heap_custom_compare) {
		return heap->cmp_fn(_1, _2);
	}
	else return 0;
}

void* t_cont_heap_create(int each_size, int cmp_type,int reserve) {
	int head_size = sizeof(continuous_heap_desc);
	int true_reserve = 1;
	while (true_reserve < reserve) {
		true_reserve = heap_next_capacity(true_reserve);
	}
	int data_size = each_size * true_reserve;
	void* heap = malloc(head_size + data_size);
	if (!heap)return heap;
	as_heap_desc(heap)->capacity = true_reserve;
	as_heap_desc(heap)->size = 0;
	as_heap_desc(heap)->each_size = each_size;
	as_heap_desc(heap)->cmp_type = cmp_type;
	return heap;
}
void* t_cont_heap_data(const void* heap) {
	return as_byte(heap) + sizeof(continuous_heap_desc);
}

int t_cont_heap_size(const void* heap) {
	return as_heap_desc(heap)->size;
}
int t_cont_heap_capacity(const void* heap) {
	return as_heap_desc(heap)->capacity;
}
int t_cont_heap_total_size(const void* heap) {
	return sizeof(continuous_heap_desc) + (as_heap_desc(heap)->capacity * as_heap_desc(heap)->each_size);
}
void t_cont_heap_set_cmp_fn(void* heap, t_cont_heap_cmp_fn_t fn) {
	as_heap_desc(heap)->cmp_fn = fn;
}
int t_cont_heap_item_size(const void* heap) {
	return as_heap_desc(heap)->each_size;
}
void* t_cont_heap_top(const void* heap) {
	return as_byte(heap) + sizeof(continuous_heap_desc);
}

void* t_cont_heap_push(void* heap, void* buf) {

	if (as_heap_desc(heap)->capacity == as_heap_desc(heap)->size) {
		heap = __t_cont_heap_expand(heap);
	}

	int cur_data_pos = heap_push_pos(heap);
	int cur_cmp_pos;
	int cmp_ret;

loop:
	if (tree_is_root(cur_data_pos)) {	// reach the root of the tree
		heap_set(heap, cur_data_pos, buf);
		as_heap_desc(heap)->size++;
		return heap;
	}

	cur_cmp_pos = tree_parent(cur_data_pos);// compare with parent node
	cmp_ret = __t_cont_heap_cmp_item(heap, buf, heap_at(heap, cur_cmp_pos));

	if (cmp_ret == -1) {// buf need to be filtered upwards
		heap_item_move(heap, cur_cmp_pos, cur_data_pos); // father move to cur_data_pos
		cur_data_pos = cur_cmp_pos;
		goto loop;
	}
	else { // buf do not need to be filtered
		heap_set(heap, cur_data_pos, buf);
		as_heap_desc(heap)->size++;
		return heap;
	}

	
}
void t_cont_heap_pop(void* heap) {
	int size = as_heap_desc(heap)->size;
	if (!size) return;
	if (size == 1) {
		as_heap_desc(heap)->size = 0;
		return;
	}

	void* compare_data = heap_at(heap, heap_last_pos(heap));

	int cur_pos = 0;
	int lcidx;
	int rcidx;
	int cmp_ret;
	int smaller_child_idx;
	int greater_child_idx;

	loop:
	lcidx = tree_lcidx(cur_pos);
	rcidx = tree_rcidx(cur_pos);

	if (lcidx >= size && rcidx >= size) {// no left & on right
		heap_set(heap, cur_pos, compare_data);
		as_heap_desc(heap)->size--;
		return;
	}
	else if (rcidx >= size) {// only left, reached the last level
		cmp_ret = __t_cont_heap_cmp_item(heap, compare_data,heap_at(heap,lcidx));
		if (cmp_ret == 1) { // need to be filtered downwards
			heap_item_move(heap, lcidx, cur_pos);
			heap_item_move(heap, cur_pos, lcidx);
			as_heap_desc(heap)->size--;
			return;
		}
		else {// not needed to be filtered
			heap_set(heap, cur_pos, compare_data);
			as_heap_desc(heap)->size--;
			return;
		}
	}
	else { // has left and right child
		cmp_ret = __t_cont_heap_cmp_item(heap, heap_at(heap,lcidx), heap_at(heap, rcidx));
		if (cmp_ret == 1) {
			greater_child_idx = lcidx;
			smaller_child_idx = rcidx;
		}
		else {
			greater_child_idx = rcidx;
			smaller_child_idx = lcidx;
		}

		cmp_ret= __t_cont_heap_cmp_item(heap, compare_data, heap_at(heap, smaller_child_idx));
		if (cmp_ret == -1) { // no need to to filter
			heap_set(heap, cur_pos, compare_data);
			as_heap_desc(heap)->size--;
			return;
		}
		else {
			heap_item_move(heap, smaller_child_idx, cur_pos);
			cur_pos = smaller_child_idx;
			goto loop;
		}
	}
	

}
void* t_cont_heap_reserve(void* heap, int size) {
	int cap = as_heap_desc(heap)->capacity;
	while (cap < size) {
		cap = heap_next_capacity(cap);
	}
	if (cap != as_heap_desc(heap)->capacity) {
		as_heap_desc(heap)->capacity = cap;
		return realloc(heap, cap * as_heap_desc(heap)->each_size);
	}
	else {
		return heap;
	}
}
int t_cont_heap_empty(const void* heap) {
	return as_heap_desc(heap)->size == 0;
}


int t_cont_heap_item_cmp_i64(void* _1, void* _2) {
	if ((*(long long*)_1) > (*(long long*)_2))return 1;
	else if ((*(long long*)_1) < (*(long long*)_2)) return -1;
	else return 0;
}
int t_cont_heap_item_cmp_u64(void* _1, void* _2) {
	if ((*(unsigned long long*)_1) > (*(unsigned long long*)_2))return 1;
	else if ((*(unsigned long long*)_1) < (*(unsigned long long*)_2)) return -1;
	else return 0;
}
int t_cont_heap_item_cmp_i32(void* _1, void* _2) {
	if ((*(int*)_1) > (*(int*)_2))return 1;
	else if ((*(int*)_1) < (*(int*)_2)) return -1;
	else return 0;
}
int t_cont_heap_item_cmp_u32(void* _1, void* _2) {
	if ((*(unsigned int*)_1) > (*(unsigned int*)_2))return 1;
	else if ((*(unsigned int*)_1) < (*(unsigned int*)_2)) return -1;
	else return 0;
}
int t_cont_heap_item_cmp_i16(void* _1, void* _2) {
	if ((*(short*)_1) > (*(short*)_2))return 1;
	else if ((*(short*)_1) < (*(short*)_2)) return -1;
	else return 0;
}
int t_cont_heap_item_cmp_u16(void* _1, void* _2) {
	if ((*(unsigned short*)_1) > (*(unsigned short*)_2))return 1;
	else if ((*(unsigned short*)_1) < (*(unsigned short*)_2)) return -1;
	else return 0;
}
int t_cont_heap_item_cmp_i8(void* _1, void* _2) {
	if ((*(char*)_1) > (*(char*)_2))return 1;
	else if ((*(char*)_1) < (*(char*)_2)) return -1;
	else return 0;
}
int t_cont_heap_item_cmp_u8(void* _1, void* _2) {
	if ((*(unsigned char*)_1) > (*(unsigned char*)_2))return 1;
	else if ((*(unsigned char*)_1) < (*(unsigned char*)_2)) return -1;
	else return 0;
}
int t_cont_heap_item_cmp_f32(void* _1, void* _2) {
	if ((*(float*)_1) > (*(float*)_2))return 1;
	else if ((*(float*)_1) < (*(float*)_2)) return -1;
	else return 0;
}
int t_cont_heap_item_cmp_f64(void* _1, void* _2) {
	if ((*(double*)_1) > (*(double*)_2))return 1;
	else if ((*(double*)_1) < (*(double*)_2)) return -1;
	else return 0;
}

int t_cont_heap_item_cmp_i64_r(void* _1, void* _2) {
	if ((*(long long*)_1) > (*(long long*)_2))return -1;
	else if ((*(long long*)_1) < (*(long long*)_2)) return 1;
	else return 0;
}
int t_cont_heap_item_cmp_u64_r(void* _1, void* _2) {
	if ((*(unsigned long long*)_1) > (*(unsigned long long*)_2))return -1;
	else if ((*(unsigned long long*)_1) < (*(unsigned long long*)_2)) return 1;
	else return 0;
}
int t_cont_heap_item_cmp_i32_r(void* _1, void* _2) {
	if ((*(int*)_1) > (*(int*)_2))return -1;
	else if ((*(int*)_1) < (*(int*)_2)) return 1;
	else return 0;
}
int t_cont_heap_item_cmp_u32_r(void* _1, void* _2) {
	if ((*(unsigned int*)_1) > (*(unsigned int*)_2))return -1;
	else if ((*(unsigned int*)_1) < (*(unsigned int*)_2)) return 1;
	else return 0;
}
int t_cont_heap_item_cmp_i16_r(void* _1, void* _2) {
	if ((*(short*)_1) > (*(short*)_2))return -1;
	else if ((*(short*)_1) < (*(short*)_2)) return 1;
	else return 0;
}
int t_cont_heap_item_cmp_u16_r(void* _1, void* _2) {
	if ((*(unsigned short*)_1) > (*(unsigned short*)_2))return -1;
	else if ((*(unsigned short*)_1) < (*(unsigned short*)_2)) return 1;
	else return 0;
}
int t_cont_heap_item_cmp_i8_r(void* _1, void* _2) {
	if ((*(char*)_1) > (*(char*)_2))return -1;
	else if ((*(char*)_1) < (*(char*)_2)) return 1;
	else return 0;
}
int t_cont_heap_item_cmp_u8_r(void* _1, void* _2) {
	if ((*(unsigned char*)_1) > (*(unsigned char*)_2))return -1;
	else if ((*(unsigned char*)_1) < (*(unsigned char*)_2)) return 1;
	else return 0;
}
int t_cont_heap_item_cmp_f32_r(void* _1, void* _2) {
	if ((*(float*)_1) > (*(float*)_2))return -1;
	else if ((*(float*)_1) < (*(float*)_2)) return 1;
	else return 0;
}
int t_cont_heap_item_cmp_f64_r(void* _1, void* _2) {
	if ((*(double*)_1) > (*(double*)_2))return -1;
	else if ((*(double*)_1) < (*(double*)_2)) return 1;
	else return 0;
}

#endif