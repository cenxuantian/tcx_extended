#include <stdio.h>
#include <t_continuous_heap.h>


int main(int argc, char** argv) {

	// create the heap
	void* heap = t_continuous_heap_creat(sizeof(int),t_continuous_heap_custom_compare,10);
	
	if (!heap) {
		printf("alloc error\n");
		exit(-1);
	}

	// set the compare function
	t_continuous_heap_set_cmp_fn(heap, t_continuous_heap_item_cmp_i32);

	printf("heap created with reserved %d space for data in sizeof %d!\n",
		t_continuous_heap_capacity(heap),
		t_continuous_heap_item_size(heap) );

	int a = 10, b = 100,c = -1 ,d = -5;
	heap = t_continuous_heap_push(heap, &a);
	heap = t_continuous_heap_push(heap, &b);
	heap = t_continuous_heap_push(heap, &c);
	heap = t_continuous_heap_push(heap, &d);
	int* res = t_continuous_heap_data(heap);
	int size = t_continuous_heap_size(heap);
	for (int i = 0; i < size; i++) {
		printf("%d ", res[i]);
	}

	free(heap);

	return 0;
}