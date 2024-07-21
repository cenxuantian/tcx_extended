#include <iostream>
#include <t_json.hpp>


using namespace tcx::json;

int main() {
	var a = new Object();
	a["1"] = "hello";
	a["2"] = 1;
	a["3"] = true;
	a["4"] = false;
	a["5"] = Array();
	a["5"].push(1, 2, 5, 5, true, false, "hello");
	a["7"] = a["2"] + 2;

	for (let i = 0; i < 10; ++i) {
		printf("%0.llf ", i.as<Number>());
	}

	if (a["3"] && a["4"]) {
		printf("yes1\n");
	}else if(a["5"][4] == true) {
		printf("yes2\n");
	}

	std::cout << JSON.stringfy(a) << '\n';
	return 0;
}