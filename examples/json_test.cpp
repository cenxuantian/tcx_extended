#include <iostream>
#include <t_json.hpp>


using namespace tcx::json;

int serialization() {
	std::string json_str = "{\"c1\":[1,2,3,4]}";
	var json = JSON.parse(json_str);
	if (json == null)  return -1;
	int num = json["c1"][0].as<Number>();
	printf("%d\n", num);
	return 0;
}

int inline_json() {
	var json = JSON.create(
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
	return 0;
}

void deserialzation(){
	var json = JSON.create(
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
	std::string json_str = JSON.stringfy(json);
	printf("%s\n", json_str.c_str());
}

var create_cmakeSettings_json() {
	var json = new Object();
	
	json["configurations"] = new Array();

	var cfg = new Object();
	cfg["name"] = "x64-Debug";
	cfg["generator"] = "Ninja";
	cfg["configurationType"] = "Debug";
	cfg["ingeritEnvironments"] = new Array{ "msvc_x64_x64" };
	cfg["buildRoot"] = "${projectDir}\\out\\build\\${name}";
	cfg["installRoot"] = "${projectDir}\\out\\install\\${name}";
	cfg["cmakeCommandArgs"] = "";
	cfg["buildCommandArgs"] = "";
	cfg["ctestCommandArgs"] = "";

	json["configurations"].push(std::move(cfg));

	return json;
}

void use_json() {
	let bob = new Object();
	let tim = new Object();

	bob["Math"] = 90;
	bob["Chinese"] = 88;
	bob["English"] = 60;

	tim["Math"] = 75;
	tim["Chinese"] = 98;
	tim["English"] = 88;

	bob["total"] = bob["Math"] + bob["Chinese"] + bob["English"];
	tim["total"] = tim["Math"] + tim["Chinese"] + tim["English"];

	if (bob["total"] > tim["total"]) printf("bob is better\n");
	else if (bob["total"] < tim["total"]) printf("tim is better\n");
	else printf("they a the same\n");

	auto is_good = [](var& person)->var {
		return person["total"] > 240;
	};

	if (is_good(bob) == true) {
		printf("bob is good\n");
	}

	if (is_good(tim) == true) {
		printf("tim is good\n");
	}
}



int main() {
	serialization();
	deserialzation();
	use_json();	

	const var a = Object({
		{"hello","yes"},
		{"good","good"},
	});
	for (const var& i : a) {
		printf("key:%s, val:%s\n",i.as<String>().c_str(), a[i].as<String>().c_str());
	}
	printf("%d\n", _MSVC_LANG);

	return 0;
}