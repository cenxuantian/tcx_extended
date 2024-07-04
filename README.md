# tcx_extended
Extension for standard library
### menu
## examples  
http_client_test.cpp  
http_test.cpp  
[sim_test.cpp](#t_simulaltionhpp)  
socket_test.cpp  
svg_test.cpp  
 
## tcx_net  
t_http.hpp  
t_http_tools.hpp  
t_sock_wrapper.hpp  
t_websocket_tools.hpp  

## tcx_pic  
## tcx_standalone  
[t_blob.hpp](#t_blobhpp)  
[t_file.hpp](#t_filehpp)  
[t_interval_map.hpp](#t_interval_maphpp)  
[t_math.hpp](#t_mathhpp)  
[t_meta.hpp](#t_metahpp)  
[t_process.hpp](#t_processhpp)  
[t_random.hpp](#t_randomhpp)  
[t_shared.hpp](#t_sharedhpp)  
[t_simulaltion.hpp](#t_simulaltionhpp)  
[t_trie.hpp](#t_triehpp)  
[t_url.hpp](#t_urlhpp)  
[t_visual.hpp](#t_visualhpp)  





### t_blob.hpp
> *binary data manager*
### t_file.hpp
> *file and path management tools*
### t_interval_map.hpp
> *template interval map implementation*
### t_math.hpp
> *for math functions and draw points*
### t_process.hpp
> *create stop resume pause and HOOK processes*
### t_random.hpp
> *random wrapper functions*
### t_shared.hpp
> *create variables in the shared memory -- shared variable on local*
### t_trie.hpp
> *trie implementations*
### t_visual.hpp
> *data visualization tools like SVG*
### t_simulaltion.hpp
> *simulation framework*  
> *you can simply use these APIs to create your simulation system*  


``` C++
// Create the Env instance
sim::Env env;
```

``` C++
// add sth. into the env
Car* car1= env.emplace_obj<Car>();
Car* car2= env.emplace_obj<Car>();
Collision* coll = env.emplace_obj<Collision>();
```

``` C++
// finally, run your env
env.run();
```


- [Interfaces](./tcx_standalone/t_simulaltion.hpp)  
- [Examples](./examples/sim_test.cpp)

