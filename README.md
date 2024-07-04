# tcx_extended
Extension for standard library
### menu
## examples  
http_client_test.cpp  
http_test.cpp  
[sim_test.cpp](./examples/sim_test.cpp)  
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
> *You can simply use these APIs to create your simulation system*  
> ***simulation module only work for c++ standards newer than `C++20`***


Here is the processes of run an env

```C++
// Create the Env instance
sim::Env env;

// add sth. into the env
Car* car1= env.emplace_obj<Car>();
Car* car2= env.emplace_obj<Car>();
Collision* coll = env.emplace_obj<Collision>();

// finally, run your env
env.run();
```

You can add anything into your environment, just make sure your classes are derived from the interface of `Object`  
You must overwrite 3 member functions -- `start()`, `stop()` and `tick(size_t step)`.

```C++
class YourClass:public sim::Object{
public:
    sim::Coro start(){}
    void stop(){}
    void tick(size_t step){}
};
```

When you call the function `Env::run()`, the `YourClass::start()` will be called.  
As you see, the `YourClass::start()` is a coroutine function. Therefore, You can use key words like `co_await`,`co_return` and `co_yield`.  
`YourClass::stop()` will be called at the time that the environment is closing.  
`YourClass::tick()` will be called on every tick of the environment.  

More information please refer to the code:
- [Interfaces](./tcx_standalone/t_simulaltion.hpp)  
- [Examples](./examples/sim_test.cpp)

