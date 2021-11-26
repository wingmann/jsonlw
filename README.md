# JsonLW
Json library for modern C++

## About
JsonLW is a lightweight Json library for exporting data in Json format from C++.
By taking advantage of templates and operator overloading on the backend,
you are able to create and work with JSON objects right away,
just as you would expect from a language such as JavaScript.
JsonLW is a single C++ Header file, "json.h".
Feel free to download this file on its own, and include it in your project.
No other requirements!

#### Platforms
JsonLW should work on any platform; it is only requirement is a C++20 compatible compiler,
as it make heavy use of the C++ move semantics, concepts and variadic templates.

One of the biggests goals for JsonLW is for it to be lightweight, and small.
Having complicated logic isn't bad, but it bloats the codebase in most cases.
I would like to keep things optimal by size rather than put in big features that take a more of space.


```cpp
#include "json.h"

int main()
{
    json::json_t obj;

    // Create a new Array as a field of an Object.
    obj["array"] = json::json_t::array(true, "Two", 3, 4.0);

    // Create a new Object as a field of another Object.
    obj["obj"] = json::json_t::object();
    
    // Assign to one of the inner object's fields
    obj["obj"]["inner"] = "Inside";
    
    // We don't need to specify the type of the JSON object:
    obj["new"]["some"]["deep"]["key"] = "Value";
    obj["array2"].append(false, "three");
    
    // We can also parse a string into a JSON object:
    obj["parsed"] = json::json_t::load("[{\"Key\": \"Value\"}, false]");
    
    std::cout << obj << '\n';
}
```
Output:
``` json
{
    "array" : [true, "Two", 3, 4.000000],
    "array2" : [false, "three"],
    "new" : {
        "some" : {
            "deep" : {
                "key" : "Value"
            }
        }
    },
    "obj" : {
        "inner" : "Inside"
    },
    "parsed" : [{ "Key" : "Value" }, false]
}
```

This example can also be written another way:
```cpp
#include "json.h"

#include <iostream>

int main()
{
    json::json_t obj {
        "array", json::json_t::array(true, "Two", 3, 4.0),
        "obj", {
            "inner", "Inside"
        },
        "new", { 
            "some", { 
                "deep", { 
                    "key", "Value" 
                } 
            } 
        },
        "array2", json::json_t::array(false, "three")
    };

    std::cout << obj << '\n';
}
```
We do not have access to the ':' character in C++, so we cannot use that to seperate key-value pairs,
but by using commas, we can achieve a similar effect.
The other point you might notice, is that we have to explictly create arrays.
This is a limitation of C++'s operator overloading rules,
so we cannot use the [] operator to define the array.
