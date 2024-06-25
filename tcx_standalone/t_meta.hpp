#pragma


namespace tcx{

template<typename Target_t, typename Original_t>
inline Target_t& refas(Original_t& item){
    return *((Target_t*)(&item));
}

template<typename Target_t, typename Original_t>
inline Target_t& derefas(Original_t* item){
    return *((Target_t*)(item));
}

}