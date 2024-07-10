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

#define rbtree_at(rbtree,idx) (as_byte(rbtree) + sizeof(continuous_rbtree_desc) + ((as_rbtree_desc(rbtree)->each_size+1)*idx) +1)

#define rbtree_is_red(rbtree,idx) ((as_byte(rbtree) + sizeof(continuous_rbtree_desc) + ((as_rbtree_desc(rbtree)->each_size+1)*idx))[0] == 1)
#define rbtree_is_black(rbtree,idx) ((as_byte(rbtree) + sizeof(continuous_rbtree_desc) + ((as_rbtree_desc(rbtree)->each_size+1)*idx))[0] == 0)
#define rbtree_is_valid(rbtree,idx) ((as_byte(rbtree) + sizeof(continuous_rbtree_desc) + ((as_rbtree_desc(rbtree)->each_size+1)*idx))[0] == 2)

// set data
#define rbtree_set(rbtree,idx,dataptr) memcpy(rbtree_at(rbtree, idx), dataptr, as_rbtree_desc(heap)->each_size)
#define rbtree_item_move(rbtree,from,to)  memmove(rbtree_at(rbtree,to), rbtree_at(rbtree, from), as_rbtree_desc(heap)->each_size)

// set colour
#define rbtree_set_black(rbtree,idx) ((rbtree_at(rbtree,idx)-1)[0] = 0)
#define rbtree_set_red(rbtree,idx) ((rbtree_at(rbtree,idx)-1)[0] = 1)
#define rbtree_set_invalid(rbtree,idx) ((rbtree_at(rbtree,idx)-1)[0] = 2)


void* __t_cont_rbtree_expand(continuous_rbtree_desc* rbtree) {
	int next = rbtree_next_capacity(rbtree->capacity);
	int size = next - rbtree->capacity;
	int start_pos = rbtree->capacity;
	rbtree->capacity = next;
	void* new_buf = realloc(as_byte(rbtree), (rbtree->capacity * (rbtree->each_size + 1)) + sizeof(continuous_rbtree_desc));
	if (!new_buf)return new_buf;
	memset(as_byte(rbtree) + start_pos, 2, size);
	return new_buf;
}
// only compare data (not compare colour)
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

void* t_cont_rbtree_create(int each_size, int cmp_type, int reserve) {
	int head_size = sizeof(continuous_rbtree_desc);
	int true_reserve = 1;
	while (true_reserve < reserve) {
		true_reserve = rbtree_next_capacity(true_reserve);
	}
	int data_size = (each_size+1)*true_reserve;
	void* rbtree = malloc(head_size + data_size);
	if (!rbtree)return rbtree;
	if (data_size) {
		memset(as_byte(rbtree)+ head_size, 2, data_size);
	}	
	as_rbtree_desc(rbtree)->capacity = true_reserve;
	as_rbtree_desc(rbtree)->size = 0;
	as_rbtree_desc(rbtree)->each_size = each_size;
	as_rbtree_desc(rbtree)->cmp_type = cmp_type;
	return rbtree;
}

void* t_cont_rbtree_emplace(void* rbtree, void* buf) {

}

void* t_cont_rbtree_at(void* rbtree, int pos) {
	return rbtree_at(rbtree, pos);
}
int t_cont_rbtree_seaarch(void* rbtree, void* data) {
	int cur_pos = 0;
	int size = as_rbtree_desc(rbtree)->size;
	if (!size) return -1;
	int cmp_ret;

	loop:
	cmp_ret = __t_cont_rbtree_cmp_item(rbtree,data, rbtree_at(rbtree,cur_pos));
	if (cmp_ret == 0) return cur_pos;
	else if (cmp_ret == 1) cur_pos = tree_rcidx(cur_pos); // go right
	else cur_pos = tree_lcidx(cur_pos);	// go left

	if (cur_pos >= size) return -1;// no child
	if (!rbtree_is_valid(rbtree, cur_pos)) return -1;// no child
	goto loop;
	return -1;
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
	return sizeof(continuous_rbtree_desc) + (as_rbtree_desc(rbtree)->capacity * (as_rbtree_desc(rbtree)->each_size+1));
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
		int start_pos = as_rbtree_desc(rbtree)->capacity;
		int size = cap - start_pos;
		as_rbtree_desc(rbtree)->capacity = cap;
		void * new_buf = realloc(rbtree, cap * (as_rbtree_desc(rbtree)->each_size + 1));
		if (!new_buf) return new_buf;
		memset(as_byte(new_buf) + start_pos, 2, size);
		return new_buf;
	}
	else {
		return rbtree;
	}
}
int t_cont_rbtree_empty(const void* rbtree) {
	return as_rbtree_desc(rbtree)->size == 0;
}

#endif