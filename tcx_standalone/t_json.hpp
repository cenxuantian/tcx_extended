/*
THIS t_json.hpp is different from what priviously posted on my profie.

THIS t_json.hpp is relay on C++ stl

*/

#include <unordered_map>
#include <vector>
#include <type_traits>
#include <string>




namespace tcx {
namespace JSON {


// predef
class var;
struct Null {};
struct Undefined {};
typedef var let;
typedef double Number;
typedef bool Boolean;
typedef std::string String;
typedef std::unordered_map<String, var> Object;
typedef std::vector<var> Array;
enum ItemType {
	OBJECT,
	ARRAY,
	BOOLEAN,
	NUMBER,
	UNDEFINED,
	J_NULL,
	STRING,
};
constexpr Null null;
constexpr Undefined undefined;

// predef static functions
ItemType typeof(var const&);
var parse(String const&);
String stringfy(var const&);
String stringfy(ItemType);
// def
// meta
#define type_eq(T1,T2) std::is_same_v<T1,T2>
#define decayT(T) std::decay_t<T>
#define __requires(...) typename = typename std::enable_if_t<__VA_ARGS__>
#define _ROUTE_TYPE(from_t, to_t,to_enum) template<> struct to_json<from_t> { inline static ItemType value; typedef to_t type; };
#define _AS(type,data) (*((type*)data))
template<typename T> struct to_json { inline static ItemType value; typedef Undefined type;};


class var {
private:
	void* data;
	ItemType type;
	friend ItemType typeof(var const&);
public:
	var();
	var(var const&);
	var(var&&);
	template<typename T>
	var(T&& data);
	template<typename T>
	var& operator=(T&&);
	var& operator=(var const&);
	var& operator= (var&&);
	~var();

	var& operator[](var const&);

};


// implementations
var::var():data(0),type(UNDEFINED){

}
var::var(var const&) {

}
var::var(var&&) {

}
template<typename T>
var::var(T&& input) {
	using input_t = std::decay_t<T>;
	using json_t1 = to_json<input_t>::type;
	if constexpr (!type_eq(json_t1, Undefined)) {
		// transfered ok
		this->type = to_json<input_t>::value;
	}
	else if constexpr(std::is_pointer_v<input_t>){
		// transfer not ok
		using input_deref_t = decltype(*(std::declval<input_t&>()));
		using json_t2 = to_json<input_deref_t>::type;
		this->type = to_json<input_deref_t>::value;

	}
	else {
		this->type = UNDEFINED;
		this->data = nullptr;
	}
}
template<typename T>
var& var::operator=(T&&) {
	return *this;
}
var& var::operator=(var const&) {
	return *this;
}
var& var::operator= (var&&) {
	return *this;
}
var::~var() {
	
}

var& var::operator[](var const&) {
	return *this;
}


ItemType typeof(var const& item) {
	return item.type;
}
String stringfy(ItemType type) {
	switch (type)
	{
	case tcx::JSON::OBJECT:return "Object";
	case tcx::JSON::ARRAY:return "Array";
	case tcx::JSON::BOOLEAN:return "Boolean";
	case tcx::JSON::NUMBER:return "Number";
	case tcx::JSON::UNDEFINED:return "Undefined";
	case tcx::JSON::J_NULL:return "Null";
	case tcx::JSON::STRING:return "String";
	default: return "Undefined";
	}
}


_ROUTE_TYPE(const char*, String, STRING);
_ROUTE_TYPE(char*, String, STRING);
_ROUTE_TYPE(String, String, STRING);
_ROUTE_TYPE(Array, Array, ARRAY);
_ROUTE_TYPE(Object, Object, OBJECT);
_ROUTE_TYPE(Undefined, Undefined, UNDEFINED);
_ROUTE_TYPE(Null, Null, J_NULL);
_ROUTE_TYPE(Boolean, Boolean, BOOLEAN);
_ROUTE_TYPE(unsigned char, Number, NUMBER);
_ROUTE_TYPE(unsigned short, Number, NUMBER);
_ROUTE_TYPE(unsigned int, Number, NUMBER);
_ROUTE_TYPE(unsigned long, Number, NUMBER);
_ROUTE_TYPE(unsigned long long, Number, NUMBER);
_ROUTE_TYPE(char, Number, NUMBER);
_ROUTE_TYPE(short, Number, NUMBER);
_ROUTE_TYPE(int, Number, NUMBER);
_ROUTE_TYPE(long, Number, NUMBER);
_ROUTE_TYPE(long long, Number, NUMBER);
_ROUTE_TYPE(float, Number, NUMBER);
_ROUTE_TYPE(double, Number, NUMBER);
_ROUTE_TYPE(long double, Number, NUMBER);




}}


#undef __requires
