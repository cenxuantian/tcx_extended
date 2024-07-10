#include <t_continuous_rbtree.h>


int main(int argc, char** argv) {
	void* map = t_cont_rbtree_create(sizeof(int), t_cont_rbtree_mem_smaller, 10);


	free(map);
	return 0;
}