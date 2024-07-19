#include <iostream>
#include <t_json.hpp>

using namespace tcx::JSON;

int main() {
	var a = false;
	to_json<bool>::type b;
	std::cout << stringfy(to_json<bool>::value) << '\n';
	return 0;
}