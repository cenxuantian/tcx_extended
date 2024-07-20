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
String stringfy(var const& value,int indent = 2,int indent_time=0);
String stringfy(ItemType);
// def
// meta
#define type_eq(T1,T2) std::is_same_v<T1,T2>
#define decayT(T) std::decay_t<T>
#define __requires(...) typename = typename std::enable_if_t<__VA_ARGS__>
#define _ROUTE_TYPE(from_t, to_t,to_enum) \
	template<> struct t_to_json<from_t> { inline static ItemType value = to_enum; typedef to_t type; };
#define _DEREF_AS(type,data) (*((type*)data))
#define _DEREF_AS_CONST(type,data) (*((const type*)data))
template<typename T> struct t_to_json { inline static ItemType value = UNDEFINED; typedef Undefined type;};
_ROUTE_TYPE(const char*, String, STRING)
_ROUTE_TYPE(char*, String, STRING)
_ROUTE_TYPE(String, String, STRING)
_ROUTE_TYPE(Array, Array, ARRAY)
_ROUTE_TYPE(Object, Object, OBJECT)
_ROUTE_TYPE(Undefined, Undefined, UNDEFINED)
_ROUTE_TYPE(Null, Null, J_NULL)
_ROUTE_TYPE(Boolean, Boolean, BOOLEAN)
_ROUTE_TYPE(unsigned char, Number, NUMBER)
_ROUTE_TYPE(unsigned short, Number, NUMBER)
_ROUTE_TYPE(unsigned int, Number, NUMBER)
_ROUTE_TYPE(unsigned long, Number, NUMBER)
_ROUTE_TYPE(unsigned long long, Number, NUMBER)
_ROUTE_TYPE(char, Number, NUMBER)
_ROUTE_TYPE(short, Number, NUMBER)
_ROUTE_TYPE(int, Number, NUMBER)
_ROUTE_TYPE(long, Number, NUMBER)
_ROUTE_TYPE(long long, Number, NUMBER)
_ROUTE_TYPE(float, Number, NUMBER)
_ROUTE_TYPE(double, Number, NUMBER)
_ROUTE_TYPE(long double, Number, NUMBER)

class var {
	friend ItemType typeof(var const&);
	friend String stringfy(var const&,int, int);
private:
	void* data_;
	ItemType type_;
public:
	var();
 	var(var const&);
	var(var&&)noexcept;
	template<typename T>
	var(T&& data);
	template<typename T>
	var& operator=(T&&);
	var& operator=(var const&);
	var& operator= (var&&) noexcept;
	~var();

	var& operator[](var const&);

	void* data();
	template<typename T, __requires(type_eq(T, typename t_to_json<T>::type))>
	T& as() { return _DEREF_AS(T, this->data_); }
	template<typename T, __requires(type_eq(T, typename t_to_json<T>::type))>
	const T & as() const{ return _DEREF_AS_CONST(T, this->data_); }
	void leak();
	void release();

	char& charAt(size_t pos);
	const char& charAt(size_t pos) const;
};


// implementations
var::var():data_(0), type_(UNDEFINED){

}
var::var(var const& other):data_(0), type_(other.type_) {
	switch (type_)
	{
	case tcx::JSON::OBJECT: 
		data_ = new Object(other.as<Object>());// copy constructor
		break;
	case tcx::JSON::ARRAY: 
		data_ = new Array(other.as<Array>());// copy constructor
		break;
	case tcx::JSON::BOOLEAN:
		data_ = new Boolean(other.as<Boolean>());// copy constructor
		break;
	case tcx::JSON::NUMBER:
		data_ = new Number(other.as<Number>());// copy constructor
		break;
	case tcx::JSON::STRING:
		data_ = new String(other.as<String>());// copy constructor
		break;
	case tcx::JSON::UNDEFINED:
		data_ = new Undefined;
		break;
	case tcx::JSON::J_NULL:
	default:break;
	}
}
var::var(var&& other)noexcept  :data_(other.data_), type_(other.type_)  {
	other.leak();
}
template<typename T>
var::var(T&& input) {
	using input_t = std::decay_t<T>;
	using json_t1 = t_to_json<input_t>::type;
	if constexpr (!type_eq(json_t1, Undefined)) {
		// transfered ok
		this->type_ = t_to_json<input_t>::value;
		this->data_ = new json_t1(std::forward<T&&>(input));
	}
	else if constexpr(std::is_pointer_v<input_t>){
		// transfer not ok
		using input_deref_t = decltype(*(std::declval<input_t&>()));
		using json_t2 = t_to_json<std::decay_t<input_deref_t>>::type;
		this->type_ = t_to_json<std::decay_t<input_deref_t>>::value;
		this->data_ = input;
	}
	else {
		this->type_ = UNDEFINED;
		this->data_ = nullptr;
	}
}
template<typename T>
var& var::operator=(T&&) {
	return *this;
}
var& var::operator=(var const& other) {
	release();
	this->type_ = other.type_;
	switch (this->type_)
	{
	case tcx::JSON::OBJECT:
		data_ = new Object(other.as<Object>());// copy constructor
		break;
	case tcx::JSON::ARRAY:
		data_ = new Array(other.as<Array>());// copy constructor
		break;
	case tcx::JSON::BOOLEAN:
		data_ = new Boolean(other.as<Boolean>());// copy constructor
		break;
	case tcx::JSON::NUMBER:
		data_ = new Number(other.as<Number>());// copy constructor
		break;
	case tcx::JSON::STRING:
		data_ = new String(other.as<String>());// copy constructor
		break;
	case tcx::JSON::UNDEFINED:
		data_ = new Undefined;
		break;
	case tcx::JSON::J_NULL:
	default:break;
	}
	return *this;
}
var& var::operator= (var&& other)noexcept {
	release();
	this->data_ = other.data_;
	this->type_ = other.type_;
	other.leak();
	return *this;
}
var::~var() {
	release();
}

var& var::operator[](var const& value) {
	switch (type_)
	{
	case tcx::JSON::OBJECT: return this->as<Object>()[value.as<String>()];
	case tcx::JSON::ARRAY: return this->as<Array>()[static_cast<size_t>( value.as<Number>())];
	case tcx::JSON::BOOLEAN:throw std::logic_error("No operator[] for typeof Boolean");
	case tcx::JSON::NUMBER: throw std::logic_error("No operator[] for typeof Number");
	case tcx::JSON::UNDEFINED:throw std::logic_error("No operator[] for typeof Undefined");
	case tcx::JSON::J_NULL:throw std::logic_error("No operator[] for typeof Null");
	case tcx::JSON::STRING:throw std::logic_error("No operator[] for typeof String. Please use charAt()");
	default: throw std::logic_error("No operator[] for typeof Unknown Type");
	}
	return *this;
}
char& var::charAt(size_t pos) {
	if (this->type_ != STRING) {
		throw std::logic_error("Cannot use function charAt() for Non-String type");
	}
	return this->as<String>()[pos];
}
const char& var::charAt(size_t pos) const {
	if (this->type_ != STRING) {
		throw std::logic_error("Cannot use function charAt() for Non-String type");
	}
	return this->as<String>()[pos];
}
void* var::data() {
	return this->data_;
}
void var::leak() {
	this->data_ = nullptr;
	this->type_ = UNDEFINED;
}
void var::release() {
	delete this->data_;
	leak();
}


ItemType typeof(var const& item) {
	return item.type_;
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
String stringfy(var const& value,int indent, int indent_time) {
	std::string ret(indent*indent_time, ' ');
	switch (value.type_)
	{
	case tcx::JSON::OBJECT: {
		ret += "{\n";
		const Object& obj = value.as<Object>();
		for (const auto& i : obj) {
			ret += '\"' + i.first +"\":"+ stringfy(i, indent, indent_time + 1) + '\n';
		}
		ret += '}';
		break;
	}
	case tcx::JSON::ARRAY: {
		ret += "[\n";
		const Array& arr = value.as<Array>();
		for (const var& i : arr) {
			ret += stringfy(i, indent, indent_time + 1) + '\n';
		}
		ret += ']';
		break;
	}
	case tcx::JSON::BOOLEAN:
		ret += value.as<Boolean>()?"true":"false";
		break;
	case tcx::JSON::NUMBER:
		ret += std::to_string(value.as<Number>());
		break;
	case tcx::JSON::STRING:
		ret += '\"' + value.as<String>()+ '\"';
		break;
	case tcx::JSON::UNDEFINED:
	case tcx::JSON::J_NULL:
	default: 
		ret += "0"; 
		break;
	}
	return ret;
}






}}


#undef __requires
