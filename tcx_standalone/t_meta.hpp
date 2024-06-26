#pragma once
#include <type_traits>

#define tcx_requires(...) typename = typename std::enable_if_t<__VA_ARGS__>

namespace tcx{

template<typename T, typename = void>
constexpr bool has_begin_v = false;

template<typename T>
constexpr bool has_begin_v<T,std::void_t<decltype(std::declval<T&>().begin())>> = true;

template<typename T, typename = void>
constexpr bool has_end_v = false;

template<typename T>
constexpr bool has_end_v<T,std::void_t<decltype(std::declval<T&>().end())>> = true;

template<typename T>
constexpr bool is_iterable_v = std::is_array_v<T> || (has_begin_v<T>&&has_end_v<T>);



template<typename Target_t, typename Original_t>
inline Target_t& refas(Original_t& item){
    return *((Target_t*)(&item));
}

template<typename Target_t, typename Original_t>
inline Target_t& derefas(Original_t* item){
    return *((Target_t*)(item));
}

}