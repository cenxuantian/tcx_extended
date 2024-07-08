#include <t_matrix.h>
#include <stdio.h>

int main() {
	t_mat mat1;
	mat1.height = 10;
	mat1.width = 10;
	mat1.each_size = sizeof(int);
	t_mat mat2;
	mat2.height = 10;
	mat2.width = 10;
	mat2.each_size = sizeof(int);

	t_mat_create(&mat1);
	t_mat_create(&mat2);

	t_mat_set_int(&mat1, 0, 0, 1);
	t_mat_set_int(&mat1, 0, 1, 2);
	t_mat_set_int(&mat2, 9, 9, 1);
	t_mat_set_int(&mat2, 8, 9, 2);
	t_mat_synchronize();


	t_mat_add_num(&mat1, 10);
	t_mat_add_num(&mat2, -3);
	t_mat_synchronize();

	t_mat_add_mat(&mat1, &mat2);
	t_mat_synchronize();

	t_mat_multiply_num(&mat1, 0.5);
	t_mat_synchronize();

	printf("mat1:\n");
	t_mat_print(&mat1);
	t_mat_synchronize();
	printf("\nmat2:\n");
	t_mat_print(&mat2);
	t_mat_synchronize();

	//t_mat_release(&mat1);
	t_mat_release(&mat2);
	return 0;
}