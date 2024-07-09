#ifndef __T_CONTINUOUS_RBTREE_C__
#define __T_CONTINUOUS_RBTREE_C__


#include "t_continuous_rbtree.h"
#include <stdlib.h>
#include <memory.h>

typedef struct {
	int each_size;
	int capacity;
	int size;
	int cmp_type;
	t_cont_rbtree_cmp_fn_t cmp_fn;
} continuous_rbtree_desc;


#define as_rbtree_desc(data)  ((continuous_rbtree_desc*)data)
#define as_byte(data) ((unsigned char*)data)

#define tree_lcidx(idx) ((idx<<1) + 1)
#define tree_rcidx(idx) ((idx<<1) + 2)
#define tree_is_rc(idx) (!(idx%2))
#define tree_is_lc(idx)	(idx%2)
#define tree_parent(idx) ((idx-1)>>1)
#define tree_is_root(idx) (idx==0)

#define rbtree_next_capacity(cur_capacity) (((cur_capacity + 1) << 1) - 1)

void* __t_cont_rbtree_expand(continuous_rbtree_desc* rbtree) {
	rbtree->capacity = rbtree_next_capacity(rbtree->capacity);
	return realloc(as_byte(rbtree), (rbtree->capacity * rbtree->each_size) + sizeof(continuous_rbtree_desc));
}

int __t_cont_rbtree_cmp_item(continuous_rbtree_desc* rbtree, void* _1, void* _2) {
	if (rbtree->cmp_type == t_cont_rbtree_mem_smaller) {
		return memcmp(_1, _2, rbtree->each_size);
	}
	else if (rbtree->cmp_type == t_cont_rbtree_mem_greater) {
		return -memcmp(_1, _2, rbtree->each_size);
	}
	else if (rbtree->cmp_type == t_cont_rbtree_custom_compare) {
		return rbtree->cmp_fn(_1, _2);
	}
	else return 0;
}

void* t_cont_rbtree_data(const void* rbtree) {
	return as_byte(rbtree) + sizeof(continuous_rbtree_desc);
}
int t_cont_rbtree_size(const void* rbtree) {
	return as_rbtree_desc(rbtree)->size;
}
int t_cont_rbtree_capacity(const void* rbtree) {
	return as_rbtree_desc(rbtree)->capacity;
}
int t_cont_rbtree_total_size(const void* rbtree) {
	return sizeof(continuous_rbtree_desc) + (as_rbtree_desc(rbtree)->capacity * as_rbtree_desc(rbtree)->each_size);
}
void t_cont_rbtree_set_cmp_fn(void* rbtree, t_cont_rbtree_cmp_fn_t fn) {
	as_rbtree_desc(rbtree)->cmp_fn = fn;
}
int t_cont_rbtree_item_size(const void* rbtree) {
	return as_rbtree_desc(rbtree)->each_size;
}
void* t_cont_rbtree_reserve(void* rbtree, int size) {
	int cap = as_rbtree_desc(rbtree)->capacity;
	while (cap < size) {
		cap = rbtree_next_capacity(cap);
	}
	if (cap != as_rbtree_desc(rbtree)->capacity) {
		as_rbtree_desc(rbtree)->capacity = cap;
		return realloc(rbtree, cap * as_rbtree_desc(rbtree)->each_size);
	}
	else {
		return rbtree;
	}
}
int t_cont_rbtree_empty(const void* rbtree) {
	return as_rbtree_desc(rbtree)->size == 0;
}

#endif