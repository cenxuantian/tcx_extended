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
[t_json_.hpp](#t_jsonhpp)  
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
### t_json.hpp
> *CPP JSON processor*  
> *Allows you use JSON as easy as it on JavaScript*
- Overview  
    This file `t_json.hpp` contains 3 main interfaces:`Classes`,`Static variables` and `Functoins`.   
    
- Create JavaScript variable  
    Using t_json, you can easily create json variables by the class `var` and its alias `let`.
    
    ```C++
    var a = 1;      // create a Number variable
    var b = true;   // create a Boolean variable
    var c = "hello world"; // craete a String variable
    var d = undefined;  // create an undefined variable
    ```
    You can also use `let` which is exactly the same as `var`.  
    To create an `Array` of `Object`, `constructor` functions needed to be called.
    Whether or not the keyword `new` is used. The class `var` will automaticly clean the space after the variable's lifecircle has ended. 
    As below: `var e = Array()` is the same as `var f = new Array()`.
    ```C++
    var e = Array();
    var f = new Array();
    var g = Object();
    var h = new Object();
    Array* arr = new Array();
    var i = arr;
    ```
- Operations  
    The class `var` supports basic operators of Number, Boolean and String. Using operators is just like using them in JavaScript.
    ```C++
    var arr = new Array();
    for (let i = 0; i < 10; ++i) {
		arr.push(i);
	}
    ```
    To access the chlidren of an `Object` or `Array`, you can just use operator[]
    ```C++
    var arr = new Array(1,"2",ture);
    var obj = new Object({{"child_0":true},{"child_1":"hello world"}});
    var arr_0 = arr[0];  //arr_0 is Number 1
    var& arr_1 = arr[1]; //arr_1 is a reference to the variable at index of 1 in the arr
    var child_0 = obj["child_0"]; // child_0 is true
    var idx = "child_1";
    var& child_1 = obj[idx]; // child_1 is a reference of variable at the index of child_1 in the obj
    obj["child_2"] = 10.5; // to emplace an item into the obj
    ```

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

