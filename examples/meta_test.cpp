#include <iostream>
#include <t_meta.hpp>
#include <t_meta_funcs.h>
#include <t_meta_num.h>

using namespace tcx::meta_num;
int main() {
	
	using a = number<10>;
	using b = number<3>;

	using c = number<6, 3>;
	using d = multiply<add<devide<a, b>, c>,number<3>>;

	std::cout << runtime::to_str<d>();

	return 0;
}