/*
THIS t_json.hpp is different from what priviously posted on my profie.

THIS t_json.hpp is relay on C++ stl

*/

/*
MIT License

Copyright (c) 2024 Cenxuan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/


#include <type_traits>
#include <unordered_map>
#include <vector>
#include <string>
#include <stack>



namespace tcx {
namespace json {


// predef

// the main variable class of JSON
class var;
// Type let is exact the same as var. It allows you to use grammar like -- let a = 10;
typedef var let;
// This class has a pre-created instance called JSON under namespace tcx::json.
// You can use tcx::json::JSON to call those functions
class JSON_GLOBAL_FUNCS;
// Type Null similar to the JavaScript
struct Null {};
// Type Undefined similar to the JavaScript, which means this variable has not been assigned any value
struct Undefined {};
typedef long double Number;
typedef bool Boolean;
typedef std::string String;
typedef std::unordered_map<String, var> Object;
typedef std::vector<var> Array;

enum ItemType {
	OBJECT		= 0,
	ARRAY		= 1,
	BOOLEAN		= 2,
	NUMBER		= 3,
	UNDEFINED	= 4,
	J_NULL		= 5,
	STRING		= 6,
};
// An global instance of class Null
constexpr Null null;
// An global instance of class Undefined
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

// meta from type to JSON type
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
	void __move_construct(var&& input) noexcept;
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
	String		stringfy(var const& value, bool format = true,int indent = 4, int indent_time = 0)const;
	String		stringfy(ItemType type)const;
	String		stringfy(Number d)const;
	var			parse(String const&)const;
	#define		create(...) parse(#__VA_ARGS__)
};
// An global instance of class JSON_GLOBAL_FUNCS
static const JSON_GLOBAL_FUNCS JSON;




/*------------------------------------------------------------------------------------------------

Interface Ends here!

------------------------------------------------------------------------------------------------*/


























// implementations
namespace {
inline bool __Number_eqaul(Number _1, Number _2, Number precision = 1.0E-10) {
	Number diff = _1 - _2;
	if (diff < 0) diff = -diff;
	return diff < precision;
}
inline bool __Is_int(Number _1) {
	return __Number_eqaul(std::round(_1), _1);
}
}


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
void var::__move_construct(var&& input)noexcept {
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
	case tcx::json::NUMBER:return __Number_eqaul(as<Number>(), input.as<Number>());
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
String JSON_GLOBAL_FUNCS::stringfy(Number d)const {
	auto ret = __Is_int(d) ? std::to_string((long long)d) : std::to_string(d);
	auto pos = ret.find_last_not_of('0');
	if (pos == String::npos) return ret;
	else return ret.substr(0, pos + 1);
}
String JSON_GLOBAL_FUNCS::stringfy(var const& value, bool format ,int indent, int indent_time)const {
	switch (value.type_)
	{
	case tcx::json::OBJECT: {
		if (format) {
			std::string ret;
			std::string end(indent * indent_time, ' ');
			end += '}';
			std::string ind(indent * (indent_time + 1), ' ');
			ret += "{\n";
			const Object& obj = value.as<Object>();
			for (const auto& i : obj) {
				ret += ind + '\"' + i.first + "\":" + stringfy(i.second, format, indent, indent_time + 1) + ",\n";
			}
			ret = ret.substr(0, ret.size() - 2);
			ret += '\n' + end;
			return ret;
		}
		else {
			const Object& obj = value.as<Object>();
			std::string ret = "{";
			for (const auto& i : obj) {
				ret += '\"' + i.first + "\":" + stringfy(i.second, format, indent, indent_time + 1) + ',';
			}
			ret = ret.substr(0, ret.size() - 1);
			ret += '}';
			return ret;
		}
	}
	case tcx::json::ARRAY: {
		if (format) {
			std::string ret;
			std::string end(indent * indent_time, ' ');
			end += ']';
			std::string ind(indent * (indent_time + 1), ' ');
			ret += "[\n";
			const Array& arr = value.as<Array>();
			for (const var& i : arr) {
				ret += ind + stringfy(i, format, indent, indent_time + 1) + ",\n";
			}
			ret = ret.substr(0, ret.size() - 2);
			ret += '\n' + end;
			return ret;
		}
		else {
			std::string ret = "[";
			const Array& arr = value.as<Array>();
			for (const var& i : arr) {
				ret += stringfy(i, format, indent, indent_time + 1) + ',';
			}
			ret = ret.substr(0, ret.size() - 1);
			ret += ']';
			return ret;
		}
	}
	case tcx::json::BOOLEAN:return value.as<Boolean>() ? "true" : "false";
	case tcx::json::NUMBER:return stringfy(value.as<Number>());
	case tcx::json::STRING:return  '\"' + value.as<String>() + '\"';
	case tcx::json::UNDEFINED:
	case tcx::json::J_NULL:
	default: return "0";
	}
}

// namespace for parser
namespace __JSON_Parser_Funcs{
static const  char __space_char_set[4] = { ' ','\t','\r','\n' };
// check if __c is space
inline bool __Is_Space(char __c) noexcept {
	for (int i = 0; i < 4; i++) {
		if (__c == __space_char_set[i]) return true;
	}
	return false;
}
// find first the first index of char which is not equal to any item in __space_char_set
inline size_t __Find_First_Not_Space(const String& str, size_t start = 0, size_t end = String::npos) {
	size_t size = str.size();
	if (!size)return String::npos;
	end = std::min(end, size - 1);
	for (size_t i = start; i <= end; i++) {
		if (!__Is_Space(str[i])) {
			return i;
		}
	}
	return String::npos;
}

inline size_t __Find_Last_Not_Space(const String& str, size_t start = 0, size_t end = String::npos) {
	size_t size = str.size();
	if (!size)return String::npos;
	end = std::min(size - 1, end);
	for (size_t i = end; i >= start; i--) {
		if (!__Is_Space(str[i])) {
			return i;
		}
	}
	return String::npos;
}

// contains start and stop
inline String __StrBetween(const String& src, size_t start, size_t stop) {
	if (start > stop) {
		return "";
	}
	return src.substr(start, stop - start + 1);
}
// the first must matched
// return the matched end position
inline size_t __Match(const String& str, char tar_start, char tar_stop, size_t cur_pos, size_t end_pos) {
	if (cur_pos >= end_pos - 1) { // not last
		return String::npos;
	}
	if (str[cur_pos] != tar_start)return String::npos;

	if (tar_start == '"') {
		for (size_t i = cur_pos + 1; i < end_pos; i++) {
			if (str[i] == tar_stop) {
				if (i == cur_pos + 1)return cur_pos + 1;
				if (str[i - 1] != '\\') {
					return i;
				}
			}
		}
		return String::npos;
	}
	else if (tar_start == '[' || tar_start == '{') {
		std::stack<size_t> record;
		record.push(cur_pos);
		for (size_t i = cur_pos + 1; i < end_pos; i++) {
			if (str[i] == tar_start) {
				record.push(i);
			}
			else if (str[i] == tar_stop) {
				if (!record.size()) {
					return String::npos;
				}
				record.pop();
				if (!record.size()) {
					return i;
				}
			}
		}
		return String::npos;
	}
	else return String::npos;


}
// the first must be a quate
inline size_t __MatchQuate(const String& str, size_t cur_pos, size_t end_pos) {
	return __Match(str, '"', '"', cur_pos, end_pos);
}
inline size_t __MatchCurlyBrace(const String& str, size_t cur_pos, size_t end_pos) {
	return __Match(str, '{', '}', cur_pos, end_pos);
}
inline size_t __MatchSquareBrace(const String& str, size_t cur_pos, size_t end_pos) {
	return __Match(str, '[', ']', cur_pos, end_pos);
}
inline bool __IsNumber(char __c) noexcept {
	if (__c >= '0' && __c <= '9')return true;
	else if (__c == '.')return true;
	else return false;
}
inline double __ToDouble(const char __c) noexcept {
	switch (__c)
	{
	case '0':return 0;
	case '1':return 1;
	case '2':return 2;
	case '3':return 3;
	case '4':return 4;
	case '5':return 5;
	case '6':return 6;
	case '7':return 7;
	case '8':return 8;
	case '9':return 9;
	default:return __c;
	}
}
var __ParseObject(const String& str, size_t& total_size, size_t& cur_pos, size_t& end_pos) {
	// obj start
	var res = Object();
read_one_member:    // lable of loop
	{
		// std::cout << res.Stringfy() << "\n";
		cur_pos = __Find_First_Not_Space(str, cur_pos + 1, end_pos);
		if (cur_pos == end_pos) {
			return res;
		}
		// read key
		if (str[cur_pos] != '"')return Null();
		size_t out_quate = __MatchQuate(str, cur_pos, end_pos); // this is the position of '"'
		if (out_quate == String::npos)return Null();
		String cur_key = __StrBetween(str, cur_pos + 1, out_quate - 1);
		if (cur_pos >= end_pos)return Null();
		cur_pos = __Find_First_Not_Space(str, out_quate + 1, end_pos);
		if (str[cur_pos] != ':') return Null();
		// read value
		cur_pos = __Find_First_Not_Space(str, cur_pos + 1, end_pos);
		if (cur_pos >= end_pos)return Null();
		char val_first_key = str[cur_pos];
		if (val_first_key == '"') {  // var is str
			out_quate = __MatchQuate(str, cur_pos, end_pos);
			if (out_quate < end_pos) {
				res[cur_key] = __StrBetween(str, cur_pos + 1, out_quate - 1);
			}
			cur_pos = out_quate + 1;
		}
		else if (val_first_key == '[') { // var is array
			out_quate = __MatchSquareBrace(str, cur_pos, end_pos);
			if (out_quate < end_pos) {
				res[cur_key] = JSON.parse(__StrBetween(str, cur_pos, out_quate));
			}
			cur_pos = out_quate + 1;
		}
		else if (val_first_key == '{') { // var is object
			out_quate = __MatchCurlyBrace(str, cur_pos, end_pos);
			if (out_quate < end_pos) {
				res[cur_key] = JSON.parse(__StrBetween(str, cur_pos, out_quate));
			}
			cur_pos = out_quate + 1;
		}
		else if (val_first_key == 'f') { // var is false
			if (((cur_pos + 4) < end_pos) && (__StrBetween(str, cur_pos, cur_pos + 4) == "false")) {
				res[cur_key] = false;
			}
			cur_pos = cur_pos + 5;
		}
		else if (val_first_key == 't') { // val is true
			if (((cur_pos + 3) <= (end_pos - 1)) && (__StrBetween(str, cur_pos, cur_pos + 3) == "true")) {
				res[cur_key] = true;
			}
			cur_pos = cur_pos + 4;
		}
		else { // val can be a number
			size_t num_start = cur_pos;
			while (__IsNumber(str[cur_pos]) && cur_pos < end_pos) {
				cur_pos++;
			}
			if (cur_pos == num_start + 1) {
				res[cur_key] = __ToDouble(str[num_start]);
			}
			else {
				try {
					res[cur_key] = std::stod(__StrBetween(str, num_start, cur_pos - 1));
				}
				catch (const std::exception&) {
					return Null();
				}
			}
		}
		if (cur_pos == end_pos)return res;
		cur_pos = __Find_First_Not_Space(str, cur_pos, end_pos);
		if (cur_pos == end_pos)return res;
		if (str[cur_pos] == ',') {
			goto read_one_member;
		}
		else return Null();
	}

}
var __ParseArray(const String& str, size_t& total_size, size_t& cur_pos, size_t& end_pos) {
	// arr start
	var res = Array();
read_one_value: // lable of loop
	cur_pos = __Find_First_Not_Space(str, cur_pos + 1, end_pos);
	if (cur_pos == end_pos) {
		return res;
	}

	char val_first_key = str[cur_pos];
	if (val_first_key == '"') {  // val is str
		size_t out_quate = __MatchQuate(str, cur_pos, end_pos);
		if (out_quate < end_pos) {
			res.push(__StrBetween(str, cur_pos + 1, out_quate - 1));
		}
		cur_pos = out_quate + 1;
	}
	else if (val_first_key == '[') { // val is array
		size_t out_quate = __MatchSquareBrace(str, cur_pos, end_pos);
		if (out_quate < end_pos) {
			res.push(JSON.parse(__StrBetween(str, cur_pos, out_quate)));
		}
		cur_pos = out_quate + 1;
	}
	else if (val_first_key == '{') { // val is object
		size_t out_quate = __MatchCurlyBrace(str, cur_pos, end_pos);
		if (out_quate < end_pos) {
			res.push(JSON.parse(__StrBetween(str, cur_pos, out_quate)));
		}
		cur_pos = out_quate + 1;
	}
	else if (val_first_key == 'f') { // val is false
		if (((cur_pos + 4) < end_pos) && (__StrBetween(str, cur_pos, cur_pos + 4) == "false")) {
			res.push(false);
		}
		cur_pos = cur_pos + 5;
	}
	else if (val_first_key == 't') { // val is true
		if (((cur_pos + 3) <= (end_pos - 1)) && (__StrBetween(str, cur_pos, cur_pos + 3) == "true")) {
			res.push(true);
		}
		cur_pos = cur_pos + 4;
	}
	else { // val can be a number
		size_t num_start = cur_pos;
		while (__IsNumber(str[cur_pos]) && cur_pos < end_pos) {
			cur_pos++;
		}
		if (cur_pos == num_start + 1) {
			res.push(__ToDouble(str[num_start]));
		}
		else {
			std::string __current_value = __StrBetween(str, num_start, cur_pos - 1);
			try {
				res.push(std::stod(__current_value));
			}
			catch (const std::exception&) {
				res.push(Null());
			}
		}
	}
	if (cur_pos == end_pos)return res;
	cur_pos = __Find_First_Not_Space(str, cur_pos, end_pos);
	if (cur_pos == end_pos)return res;
	if (str[cur_pos] == ',') {
		goto read_one_value;
	}
	else return Null();
}

}

var JSON_GLOBAL_FUNCS::parse(String const& str)const {
	// set local-global value
	size_t total_size = str.size();
	if (total_size < 2) return Null();   // check least length
	size_t cur_pos = __JSON_Parser_Funcs::__Find_First_Not_Space(str);   // find first valid pos
	size_t end_pos = __JSON_Parser_Funcs::__Find_Last_Not_Space(str);    // find last valid pos
	if (cur_pos == String::npos || end_pos == String::npos || cur_pos >= end_pos) return Null();
	
	if (str[cur_pos] == '{' && str[end_pos] == '}')
		return __JSON_Parser_Funcs::__ParseObject(str, total_size, cur_pos, end_pos);   // this is an object
	else if (str[cur_pos] == '[' && str[end_pos] == ']')   // this is an array
		return __JSON_Parser_Funcs::__ParseArray(str, total_size, cur_pos, end_pos);
	else return Null();

	return Null();
}



}}


#undef __requires
#undef type_eq
#undef decayT
#undef _ROUTE_TYPE
#undef _DEREF_AS
#undef _DEREF_AS_CONST