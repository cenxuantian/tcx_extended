#pragma once
#include <string>

#define __member static constexpr
namespace tcx
{
// private:
namespace{
using i64=long long;
template<i64 _1>
constexpr i64 i64_abs_v = _1 < 0? -_1:_1;
template<i64 _1, i64 _2>
constexpr i64 i64_mod_v = _2==0? _1:_1%_2;
template<i64 _1, i64 _2>
constexpr i64 i64_gcd_f(){
    if constexpr(_1==0||_2==0) return 0;
    else if constexpr(i64_abs_v<_1> == i64_abs_v<_2>) return i64_abs_v<_1>;
    else if constexpr(i64_abs_v<_1> > i64_abs_v<_2>){
        if constexpr(i64_mod_v<_1,_2> == 0) return i64_abs_v<_2>;
        else return i64_gcd_f<_1%_2,_2>();
    }else{
        if constexpr(i64_mod_v<_2,_1> ==0) return i64_abs_v<_1>;
        else return i64_gcd_f<_2%_1,_1>();
    }
};
template<i64 _1, i64 _2>
constexpr i64 i64_lcm_f(){
    if constexpr(_1==0||_2==0) return 0;
    return _1/i64_gcd_f<_1,_2>()*_2;
}

template<typename T, typename = void>
constexpr bool __has_member_num_v = false;
template<typename T>
constexpr bool __has_member_num_v<T,std::void_t<decltype(T::num)>> = true;
template<typename T, typename = void>
constexpr bool __has_member_den_v = false;
template<typename T>
constexpr bool __has_member_den_v<T,std::void_t<decltype(T::den)>> = true;
template<typename T>
constexpr bool __is_number_v = __has_member_den_v<T> && __has_member_num_v<T>;
template<typename T>
constexpr bool __is_integer_f(){
    if constexpr (__is_number_v<T>){
        if constexpr (T::den == 1) return true;
        else return false;
    }else return false;
}
template<typename T>
constexpr bool __is_decimal_f(){
    if constexpr (__is_number_v<T>){
        if constexpr (T::den != 1) return true;
        else return false;
    }else return false;
}
template<typename T>
constexpr bool __is_valid_number_type_v = __is_number_v<T>||std::is_integral_v<T>;

}
// public:
namespace meta_num{

template<i64 _num,i64 _den = 1>
struct number{
    __member i64 num = _num==0? 0:_num / i64_gcd_f<_num,_den>();
    __member i64 den = _num==0? _den:_den / i64_gcd_f<_num,_den>();
};

template<typename T>
constexpr bool is_number_v = __is_number_v<T>;
template<typename T>
constexpr bool is_integer_v = __is_integer_f<T>();
template<typename T>
constexpr bool is_decimal_v = __is_decimal_f<T>();

template<typename _t1,typename _t2>
using add_two = number<((i64_lcm_f<_t1::den,_t2::den>()/_t1::den) * _t1::num) + ((i64_lcm_f<_t1::den,_t2::den>()/_t2::den) * _t2::num),i64_lcm_f<_t1::den,_t2::den>()>;
template<typename _t1,typename ...types>
struct add_all { using type = add_two<_t1, typename add_all<types...>::type>; };
template<typename T>
struct add_all<T> {using type = typename T;};
template<typename _t1,typename ...types>
using add = typename add_all<_t1, types...>::type;

template<typename _t1,typename _t2>
using minus_two = number<((i64_lcm_f<_t1::den,_t2::den>()/_t1::den) * _t1::num) - ((i64_lcm_f<_t1::den,_t2::den>()/_t2::den) * _t2::num),i64_lcm_f<_t1::den,_t2::den>()>;
template<typename _t1, typename ...types>
struct minus_all { using type = minus_two<_t1, typename minus_all<types...>::type>; };
template<typename T>
struct minus_all<T> { using type = typename T; };
template<typename _t1, typename ...types>
using minus = typename minus_all<_t1, types...>::type;

template<typename _t1,typename _t2>
using multiply_two = number<_t1::num*_t2::num,_t1::den*_t2::den>;
template<typename T, typename ...types>
struct multiply_all{using type = multiply_two<T,typename multiply_all<types...>::type>;};
template<typename T>
struct multiply_all<T>{using type = T;};
template<typename T, typename ...types>
using multiply = typename multiply_all<T,types...>::type;

template<typename _t1,typename _t2>
using divide_two = number<_t1::num*_t2::den,_t1::den*_t2::num>;
template<typename T, typename ...types>
struct divide_all{using type = divide_two<T,typename divide_all<types...>::type>;};
template<typename T>
struct divide_all<T>{using type = T;};
template<typename T, typename ...types>
using devide = typename divide_all<T,types...>::type;





namespace runtime {

template<typename T, typename = typename std::enable_if_t<is_number_v<T>>>
constexpr double to_double() {
    return static_cast<double>(T::num) / static_cast<double>(T::den);
}

template<typename T, typename = typename std::enable_if_t<is_number_v<T>>>
constexpr std::string to_str() {
    if constexpr (is_decimal_v<T>) {
        return std::to_string(T::num) + "/" + std::to_string(T::den);
    }
    else {
        return std::to_string(T::num);
    }
}

template<typename T, typename = typename std::enable_if_t<is_number_v<T>>>
constexpr double to_double(T const&) {
    return static_cast<double>(T::num) / static_cast<double>(T::den);
}

template<typename T, typename = typename std::enable_if_t<is_number_v<T>>>
constexpr std::string to_str(T const&) {
    if constexpr (is_decimal_v<T>) {
        return std::to_string(T::num) + "/" + std::to_string(T::den);
    }
    else {
        return std::to_string(T::num);
    }
}

}


}

} // namespace tcx
