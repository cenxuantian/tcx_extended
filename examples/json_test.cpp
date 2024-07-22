#include <iostream>
#include <t_json.hpp>


using namespace tcx::json;

int main() {
	var a = JSON.create(
		{
			"c1":"1",
			"c2" : [1,2.1,3,4,"hello",{
				"cc1": [true,false] ,
				"cc2" : ["yes","hello"] ,
				"cc3" : false
			}] ,
			"c3" : 0,
			"c4" : {
				"cc1":"hello",
				"cc2" : 1
			}
		}
	);

	if (a == null) {
		printf("JSON error\n");
		return -1;
	}
	var b = JSON.parse(JSON.stringfy(a,false));
	
	if (a == b) {
		printf("yes");
		printf("\n%s\n", JSON.stringfy(b,true,8).c_str());
	}

	return 0;
}