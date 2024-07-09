#include <stdio.h>
#include <t_continuous_heap.h>


void print_heap(void* heap) {
	int size = t_cont_heap_size(heap);
	int* data = t_cont_heap_data(heap);
	
	int cur_count = 1;
	int j = 0;
	for (int i = 0; i < size; i++) {
		printf("%d\t", data[i]);
		j++;
		if (j == cur_count) {
			j = 0;
			printf("\n");
			cur_count <<= 1;
		}
	}
	printf("\n---------------------------------------------------------------------------------------------\n");
}

int main(int argc, char** argv) {

	// create the heap
	void* heap = t_cont_heap_create(sizeof(int),t_cont_heap_custom_compare,10);
	
	if (!heap) {
		printf("alloc error\n");
		exit(-1);
	}

	// set the compare function
	t_cont_heap_set_cmp_fn(heap, t_cont_heap_item_cmp_i32);

	printf("heap created with reserved %d space for data in sizeof %d!\n",
		t_cont_heap_capacity(heap),
		t_cont_heap_item_size(heap) );

	int a = 10, b = 100,c = -1 ,d = -5,e= 11,f= -16;
	// you can use function to push
	heap = t_cont_heap_push(heap, &a);
	heap = t_cont_heap_push(heap, &e);
	heap = t_cont_heap_push(heap, &c);
	heap = t_cont_heap_push(heap, &a);
	heap = t_cont_heap_push(heap, &a);
	heap = t_cont_heap_push(heap, &c);
	heap = t_cont_heap_push(heap, &b);
	heap = t_cont_heap_push(heap, &a);
	heap = t_cont_heap_push(heap, &c);
	heap = t_cont_heap_push(heap, &f);
	heap = t_cont_heap_push(heap, &a);
	heap = t_cont_heap_push(heap, &f);
	heap = t_cont_heap_push(heap, &b);
	heap = t_cont_heap_push(heap, &a);
	heap = t_cont_heap_push(heap, &e);
	heap = t_cont_heap_push(heap, &a);
	heap = t_cont_heap_push(heap, &a);
	heap = t_cont_heap_push(heap, &e);
	heap = t_cont_heap_push(heap, &b);
	heap = t_cont_heap_push(heap, &c);
	heap = t_cont_heap_push(heap, &e);
	heap = t_cont_heap_push(heap, &d);
	// you can use macro to push rvalue or lvalue
	t_cont_heap_push_lv(heap, a);
	t_cont_heap_push_lv(heap, b);
	t_cont_heap_push_lv(heap, c);
	t_cont_heap_push_lv(heap, d);
	t_cont_heap_push_lv(heap, a);
	t_cont_heap_push_lv(heap, b);
	t_cont_heap_push_lv(heap, c);
	t_cont_heap_push_lv(heap, d);
	//
	t_cont_heap_push_rv(heap, int, 7);
	t_cont_heap_push_rv(heap, int, 6);
	t_cont_heap_push_rv(heap, int, 5);
	t_cont_heap_push_rv(heap, int, 4);
	t_cont_heap_push_rv(heap, int, 3);
	t_cont_heap_push_rv(heap, int, 2);
	t_cont_heap_push_rv(heap, int, 1);
	t_cont_heap_push_rv(heap, int, -1);
	t_cont_heap_push_rv(heap, int, -1);
	t_cont_heap_push_rv(heap, int, -15);
	t_cont_heap_push_rv(heap, int, -16);
	t_cont_heap_push_rv(heap, int, -17);

	while (!t_cont_heap_empty(heap)) {
		printf("%d ", t_cont_heap_top_as(heap,int));
		t_cont_heap_pop(heap);
	}

	free(heap);

	return 0;
}