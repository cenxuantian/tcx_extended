#pragma once
#include <type_traits>
#include <cstdint>

namespace tcx
{

template<typename T>
using rm_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template<typename T>
using root_t = std::remove_pointer_t<std::decay_t<T>>;

template<typename number_type,number_type number_value>
struct num_t{
    static constexpr number_type value = number_value;
};

template<typename T>
constexpr size_t get_arr_dimension = std::is_array_v<T>? get_arr_dimension<std::remove_extent_t<T>>+1:0;

template<typename T>
constexpr size_t get_pointer_dimension = std::is_pointer_v<T>? get_pointer_dimension<std::remove_pointer_t<T>>+1:0;

template<typename T>
constexpr size_t get_arr_length = std::is_array_v<T>? sizeof(T)/sizeof(std::remove_extent_t<T>):0;
template<typename T>
constexpr size_t get_arr_length<T[]> = 0;

template <typename _Tp>
constexpr
decltype(std::declval<std::remove_reference_t<_Tp>&>(0)) 
decl_lvref() noexcept;

template <typename _Tp>
constexpr
decltype(std::declval<std::remove_reference_t<_Tp>&&>(0))
decl_rvref() noexcept;

template <typename _Tp>
constexpr
decltype(std::declval<std::conditional_t<std::is_rvalue_reference_v<_Tp>,std::remove_reference_t<_Tp>&&,std::remove_reference_t<_Tp>&>>(0))
declval_forward() noexcept;


template<typename number_type,number_type first_number, number_type ...numbers>
struct __meta_array
{
    using content_t = number_type;
    using prev_t = __meta_array<number_type,numbers...>;
    static constexpr number_type value  = num_t<number_type,first_number>::value;
};

template<typename number_type,number_type first_number>
struct __meta_array<number_type,first_number>
{
    using content_t = number_type;
    static constexpr number_type value  = num_t<number_type,first_number>::value;
};


template<typename number_type,number_type ...numbers>
struct meta_array
{
    using data_t = __meta_array<number_type,numbers...>;
    using content_t = number_type;
    static constexpr size_t size = sizeof...(numbers);
};

template<typename number_type>
struct meta_array<number_type>
{
    using content_t = number_type;
    static constexpr size_t size = 0;
};

template<typename T,typename = void>
constexpr bool has_prev_t_v = false;
template<typename T>
constexpr bool has_prev_t_v<T,std::void_t<typename T::prev_t>> = true;

template<typename T,typename = void>
constexpr bool has_data_t_v = false;
template<typename T>
constexpr bool has_data_t_v<T,std::void_t<typename T::data_t>> = true;

template<size_t idx,typename T>
constexpr auto get(){
    if constexpr (has_data_t_v<T>){
        static_assert(idx < T::size,"idx out of range!");
        return get<idx,typename T::data_t>();
    }
    else if constexpr (idx == 0){
        return T::value;
    }else{
        return get<idx-1,typename T::prev_t>();
    }
}

template<typename T, T _1>
constexpr T abs_v = std::is_signed_v<T>&& _1<0? -_1:_1;
template<typename T,T _1,T _2>
constexpr T min_v = _1>_2? _2:_1;
template<typename T,T _1,T _2>
constexpr T max_v = _1>_2? _1:_2;
template<typename T>
constexpr T zero_v = 0;
template<typename T, T _value>
struct numberof{
    constexpr static T value= _value;
};

template<typename T, T _1,T _2>
constexpr T mod(){
    if constexpr (_2==0) return _1;
    else return _1%_2;
}

template<typename T,T first,T ...others>
constexpr T __get_first = first;

template<typename T, T _1,T _2>
constexpr T gcd(){
    if constexpr(_1==0||_2==0){
        return zero_v<T>;
    }else if constexpr(abs_v<T,_1> == abs_v<T,_2> ){
        return abs_v<T,_1>;
    }else if constexpr(abs_v<T,_1> > abs_v<T,_2> ){
        if constexpr(mod<T,_1,_2>()==0) return abs_v<T,_2>;
        else return __gcd_of_2<T,_1%_2,_2>();
    }else{
        if constexpr(mod<T,_2,_1>()==0) return abs_v<T,_1>;
        else return __gcd_of_2<T,_2%_1,_1>();
    }
    return zero_v<T>;
}
template<typename T,T _1,T _2>
constexpr T lcm(){
    if constexpr (_1==0||_2==0){
        return zero_v<T>;
    }else{
        return _1*_2/gcd<_1,_2>;
    }
}


template<long long _numerator,long long _denominator = 1>
struct fraction{
    static constexpr long long numerator = _numerator/gcd<long long,_numerator,_denominator>();
    static constexpr long long denominator = _denominator/gcd<long long,_numerator,_denominator>();
};


template<typename T1,typename T2>
constexpr auto fenshu_devid(){ return fenshu<T1::up*T2::down,T1::down*T2::up>;}
template<typename T1,typename T2>
constexpr auto fenshu_multi(){ return fenshu<T1::up*T2::up,T1::down*T2::down>;}



} // namespace tcx
