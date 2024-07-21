/*
THIS t_json.hpp is different from what priviously posted on my profie.

THIS t_json.hpp is relay on C++ stl

*/

#include <unordered_map>
#include <vector>
#include <type_traits>
#include <string>




namespace tcx {
namespace json {


// predef
class var;
class JSON_GLOBAL_FUNCS;
struct Null {};
struct Undefined {};
typedef var let;
typedef long double Number;
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
	friend class JSON_GLOBAL_FUNCS;
	friend ItemType typeof(var const&);
private:
	void* data_;
	ItemType type_;
private:
	void __default_construct();
	void __copy_construct(const var& input);
	void __move_construct(var&& input);
	template<typename T> void __any_constructor(T&& input);

public:
	// constructor
	var();
 	var(var const&);
	var(var&&)noexcept;
	template<typename T> var(T&& data);
	
	// assignment
	var& operator=(var const&);
	var& operator= (var&&) noexcept;
	template<typename T> var& operator=(T&&);

	// destructor
	~var();

	// get data
	var& operator[](var const&);
	void* data();


	// type cast
	template<typename T, __requires(type_eq(T, typename t_to_json<T>::type))> T& as() { return _DEREF_AS(T, this->data_); }
	template<typename T, __requires(type_eq(T, typename t_to_json<T>::type))> const T & as() const{ return _DEREF_AS_CONST(T, this->data_); }
	
	// release
	void leak();
	void release();

	// Array func
	template<typename first_t, typename ...arg_t> void push(first_t&& first, arg_t&& ...items);

	// Object func
	Boolean hasOwnProperty(var const&) const;

	// String func
	char& charAt(size_t pos);
	const char& charAt(size_t pos) const;

	// basic operation
	size_t length();
	var operator+(const var&) const;
	var operator-(const var&) const;
	var operator*(const var&) const;
	var operator/(const var&) const;
	var& operator+=(const var&);
	var& operator-=(const var&);
	var& operator*=(const var&);
	var& operator/=(const var&);
	var operator%(const var&)const;
	Boolean operator==(const var&) const noexcept;
	Boolean operator!=(const var& ) const noexcept;
	Boolean operator>(const var&)const;
	Boolean operator>=(const var&)const;
	Boolean operator<(const var&)const;
	Boolean operator<=(const var&)const;
	Boolean operator!()const noexcept;
	Boolean operator&&(const var&)const noexcept;
	Boolean operator||(const var&)const noexcept;
	var& operator++();
	const var operator++(int);
};

class JSON_GLOBAL_FUNCS {
public:
	String stringfy(var const& value, int indent = 2, int indent_time = 0)const;
	String stringfy(ItemType type)const;
	var parse(String const&)const;
};
static const JSON_GLOBAL_FUNCS JSON;

































// implementations
void var::__default_construct() {
	this->type_ = UNDEFINED;
	this->data_ = nullptr;
}
void var::__copy_construct(const var& input) {
	this->type_ = input.type_;
	switch (this->type_)
	{
	case tcx::json::OBJECT:
		data_ = new Object(input.as<Object>());// copy constructor
		break;
	case tcx::json::ARRAY:
		data_ = new Array(input.as<Array>());// copy constructor
		break;
	case tcx::json::BOOLEAN:
		data_ = new Boolean(input.as<Boolean>());// copy constructor
		break;
	case tcx::json::NUMBER:
		data_ = new Number(input.as<Number>());// copy constructor
		break;
	case tcx::json::STRING:
		data_ = new String(input.as<String>());// copy constructor
		break;
	case tcx::json::UNDEFINED:
		data_ = new Undefined;
		break;
	case tcx::json::J_NULL:
	default:break;
	}
}
void var::__move_construct(var&& input) {
	this->type_ = input.type_;
	this->data_ = input.data_;
	input.leak();
}
template<typename T> void var::__any_constructor(T&& input) {
	using input_t = std::decay_t<T>;
	using json_t1 = t_to_json<input_t>::type;
	if constexpr (!type_eq(json_t1, Undefined)) {// type transfered ok
		this->type_ = t_to_json<input_t>::value;
		this->data_ = new json_t1(std::forward<T&&>(input));
	}
	else if constexpr (std::is_pointer_v<input_t>) {// transfer not ok
		using input_deref_t = decltype(*(std::declval<input_t&>()));
		using json_t2 = t_to_json<std::decay_t<input_deref_t>>::type;
		this->type_ = t_to_json<std::decay_t<input_deref_t>>::value;
		this->data_ = input;
	}
	else if constexpr (type_eq(const var&, T) || type_eq(var&, T)) __copy_construct(input);
	else if constexpr (type_eq(var&&, T))__move_construct(std::forward<var&&>(input));
	else __default_construct();
}


var::var(){
	__default_construct();
}
var::var(var const& input) {
	__copy_construct(input);
}
var::var(var&& input)noexcept {
	__move_construct(std::forward<var&&>(input));
}
template<typename T> var::var(T&& input) {
	__any_constructor(std::forward<T&&>(input));
}
template<typename T> var& var::operator=(T&& input) {
	release();
	__any_constructor(std::forward<T&&>(input));
	return *this;
}
var& var::operator=(var const& other) {
	release();
	__copy_construct(other);
	return *this;
}
var& var::operator= (var&& other)noexcept {
	release();
	__move_construct(std::forward<var&&>(other));
	return *this;
}
var::~var() {
	release();
}

var& var::operator[](var const& value) {
	switch (type_)
	{
	case tcx::json::OBJECT: return this->as<Object>()[value.as<String>()];
	case tcx::json::ARRAY: return this->as<Array>()[static_cast<size_t>( value.as<Number>())];
	case tcx::json::BOOLEAN:throw std::logic_error("No operator[] for typeof Boolean");
	case tcx::json::NUMBER: throw std::logic_error("No operator[] for typeof Number");
	case tcx::json::UNDEFINED:throw std::logic_error("No operator[] for typeof Undefined");
	case tcx::json::J_NULL:throw std::logic_error("No operator[] for typeof Null");
	case tcx::json::STRING:throw std::logic_error("No operator[] for typeof String. Please use charAt()");
	default: throw std::logic_error("No operator[] for typeof Unknown Type");
	}
	return *this;
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

// Array func
template<typename first_t, typename ...arg_t> void var::push(first_t&& first, arg_t&& ...items) {
	if (this->type_ != ARRAY) {
		throw std::logic_error("using push, target must be Array type");
	}
	this->as<Array>().emplace_back(std::forward<first_t&&>(first));
	if constexpr (sizeof...(arg_t)>0) {
		this->push(std::forward<arg_t&&>(items)...);
	}
}

// Object func
Boolean var::hasOwnProperty(var const& key)const {
	if (typeof(key) != STRING) {
		throw std::logic_error("using hasOwnProperty, key must be String type");
	}
	else if (this->type_ != OBJECT) {
		throw std::logic_error("using hasOwnProperty, target must be Object type");
	}
	else return this->as<Object>().count(key.as<String>());
}

// String func
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

size_t var::length() {
	switch (type_)
	{
	case tcx::json::OBJECT:return as<Object>().size();
	case tcx::json::ARRAY:return as<Array>().size();
	case tcx::json::STRING: return as<String>().size();
	case tcx::json::BOOLEAN:
	case tcx::json::NUMBER:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: return ~0ULL;
	}
}
var var::operator+(const var& input) const {
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() + input.as<Number>();
	case tcx::json::STRING: return as<String>() + input.as<String>();
	case tcx::json::OBJECT:
	case tcx::json::ARRAY:
	case tcx::json::BOOLEAN:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return {};
}
var var::operator-(const var& input) const {
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() - input.as<Number>();
	case tcx::json::STRING:
	case tcx::json::OBJECT:
	case tcx::json::ARRAY:
	case tcx::json::BOOLEAN:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return {};
}
var var::operator*(const var& input) const {
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() * input.as<Number>();
	case tcx::json::STRING:
	case tcx::json::OBJECT:
	case tcx::json::ARRAY:
	case tcx::json::BOOLEAN:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return {};
}
var var::operator/(const var& input) const {
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() / input.as<Number>();
	case tcx::json::STRING:
	case tcx::json::OBJECT:
	case tcx::json::ARRAY:
	case tcx::json::BOOLEAN:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return {};
}
var& var::operator+=(const var& input)  {
	switch (type_)
	{
	case tcx::json::NUMBER:as<Number>() += input.as<Number>(); break;
	case tcx::json::STRING: as<String>() += input.as<String>(); break;
	case tcx::json::OBJECT:
	case tcx::json::ARRAY:
	case tcx::json::BOOLEAN:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return *this;
}
var& var::operator-=(const var& input)  {
	switch (type_)
	{
	case tcx::json::NUMBER:as<Number>() -= input.as<Number>(); break;
	case tcx::json::STRING:
	case tcx::json::OBJECT:
	case tcx::json::ARRAY:
	case tcx::json::BOOLEAN:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return *this;
}
var& var::operator*=(const var& input)  {
	switch (type_)
	{
	case tcx::json::NUMBER:as<Number>() *= input.as<Number>(); break;
	case tcx::json::STRING:
	case tcx::json::OBJECT:
	case tcx::json::ARRAY:
	case tcx::json::BOOLEAN:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return *this;
}
var& var::operator/=(const var& input)  {
	switch (type_)
	{
	case tcx::json::NUMBER:as<Number>() /= input.as<Number>(); break;
	case tcx::json::STRING:
	case tcx::json::OBJECT:
	case tcx::json::ARRAY:
	case tcx::json::BOOLEAN:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return *this;
}
var var::operator%(const var& input)const {
	switch (type_)
	{
	case tcx::json::NUMBER:return std::fmod<Number>(as<Number>(), input.as<Number>());
	case tcx::json::STRING:
	case tcx::json::OBJECT:
	case tcx::json::ARRAY:
	case tcx::json::BOOLEAN:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return {};
}
Boolean var::operator==(const var& input) const noexcept {
	if (typeof(*this) != typeof(input))return false;
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() == input.as<Number>();
	case tcx::json::STRING:return as<String>() == input.as<String>();
	case tcx::json::OBJECT:return as<Object>() == input.as<Object>();
	case tcx::json::ARRAY:return as<Array>() == input.as<Array>();
	case tcx::json::BOOLEAN:return as<Boolean>() == input.as<Boolean>();
	case tcx::json::UNDEFINED:return true;
	case tcx::json::J_NULL:return true;
	default: return false;
	}
}
Boolean var::operator!=(const var& input) const noexcept {
	return !((*this) == input);
}
Boolean var::operator>(const var& input)const {
	if (typeof(*this) != typeof(input))return false;
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() > input.as<Number>();
	case tcx::json::STRING:return as<String>() > input.as<String>();
	case tcx::json::BOOLEAN:return as<Boolean>() > input.as<Boolean>();
	case tcx::json::ARRAY: return as<Array>() > input.as<Array>();
	case tcx::json::OBJECT:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return false;
}
Boolean var::operator>=(const var& input)const {
	if (typeof(*this) != typeof(input))return false;
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() >= input.as<Number>();
	case tcx::json::STRING:return as<String>() >= input.as<String>();
	case tcx::json::BOOLEAN:return as<Boolean>() >= input.as<Boolean>();
	case tcx::json::ARRAY: return as<Array>() >= input.as<Array>();
	case tcx::json::OBJECT:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return false;
}
Boolean var::operator<(const var& input)const {
	if (typeof(*this) != typeof(input))return false;
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() < input.as<Number>();
	case tcx::json::STRING:return as<String>() < input.as<String>();
	case tcx::json::BOOLEAN:return as<Boolean>() < input.as<Boolean>();
	case tcx::json::ARRAY: return as<Array>() < input.as<Array>();
	case tcx::json::OBJECT:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return false;
}
Boolean var::operator<=(const var& input)const {
	if (typeof(*this) != typeof(input))return false;
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() <= input.as<Number>();
	case tcx::json::STRING:return as<String>() <= input.as<String>();
	case tcx::json::BOOLEAN:return as<Boolean>() <= input.as<Boolean>();
	case tcx::json::ARRAY: return as<Array>() <= input.as<Array>();
	case tcx::json::OBJECT:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return false;
}
Boolean var::operator!()const noexcept {
	switch (type_)
	{
	case tcx::json::NUMBER:return as<Number>() ==0;
	case tcx::json::STRING:return as<String>().size() == 0;
	case tcx::json::BOOLEAN:return !as<Boolean>();
	case tcx::json::ARRAY: return as<Array>().size() == 0;
	case tcx::json::OBJECT:return as<Object>().size() == 0;
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: return true;
	}
}
Boolean var::operator&&(const var& input)const noexcept {
	return (!(*this)) && (!input);
}
Boolean var::operator||(const var& input)const noexcept {
	return (!(*this)) || (!input);
}
var& var::operator++() {
	switch (type_)
	{
	case tcx::json::NUMBER: ++as<Number>(); break;
	case tcx::json::STRING:
	case tcx::json::BOOLEAN:
	case tcx::json::ARRAY:
	case tcx::json::OBJECT:
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: throw std::logic_error("Error in function: " __FUNCTION__ " typeof function caller is " + JSON.stringfy(type_));
	}
	return *this;
}
const var var::operator++(int) {
	var ret = (*this);
	++(*this);
	return ret;
}

ItemType typeof(var const& item) {
	return item.type_;
}
String JSON_GLOBAL_FUNCS::stringfy(ItemType type)const {
	switch (type)
	{
	case tcx::json::OBJECT:return "Object";
	case tcx::json::ARRAY:return "Array";
	case tcx::json::BOOLEAN:return "Boolean";
	case tcx::json::NUMBER:return "Number";
	case tcx::json::UNDEFINED:return "Undefined";
	case tcx::json::J_NULL:return "Null";
	case tcx::json::STRING:return "String";
	default: return "Undefined";
	}
}
String JSON_GLOBAL_FUNCS::stringfy(var const& value,int indent, int indent_time)const {
	switch (value.type_)
	{
	case tcx::json::OBJECT: {
		std::string ret(indent * indent_time, ' ');
		std::string end = ret + '}';
		std::string ind(indent * (indent_time + 1), ' ');
		ret += "{\n";
		const Object& obj = value.as<Object>();
		for (const auto& i : obj) {
			ret += ind+ '\"' + i.first +"\":"+ stringfy(i.second, indent, indent_time + 1) + ",\n";
		}
		ret = ret.substr(0,ret.size() - 2);
		ret += '\n'+ end;
		return ret;
	}
	case tcx::json::ARRAY: {
		std::string ret(indent * indent_time, ' ');
		std::string end = ret + ']';
		std::string ind(indent * (indent_time + 1), ' ');
		ret += "[\n";
		const Array& arr = value.as<Array>();
		for (const var& i : arr) {
			ret += ind+ stringfy(i, indent, indent_time + 1) +",\n";
		}
		ret = ret.substr(0, ret.size() - 2);
		ret += '\n' + end;
		return ret;
	}
	case tcx::json::BOOLEAN:return value.as<Boolean>() ? "true" : "false";
	case tcx::json::NUMBER:return std::to_string(value.as<Number>());
	case tcx::json::STRING:return  '\"' + value.as<String>() + '\"';
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: return "0";
	}
}
var JSON_GLOBAL_FUNCS::parse(String const&)const {

	return Null{};
}





}}


#undef __requires
