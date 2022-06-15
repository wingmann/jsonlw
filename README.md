# JsonLW
Json library for modern C++

### About
JsonLW is a lightweight Json library for exporting data in Json format from C++.\
By taking advantage of templates and operator overloading on the backend,
you are able to create and work with Json objects right away,
just as you would expect from a language such as JavaScript.
JsonLW is a single C++ Header file, "json.h".
Feel free to download this file on its own, and include it in your project.
No other requirements!

### Platforms
JsonLW should work on any platform.\
One of the biggest goals for JsonLW is for it to be lightweight, and small.
Having complicated logic isn't bad, but it bloats the codebase in most cases.
I would like to keep things optimal by size rather than put in big features that take a more of space.

### Examples
```cpp
#include "json.h"

#include <iostream>

using namespace wingmann;

int main() {
    json obj;

    // Create a new array as a field of an object.
    obj["array"] = json::array(true, "two", 3, 4.0);

    // Create a new object as a field of another object.
    obj["obj"] = json::object();
    
    // Assign to one of the inner object's fields
    obj["obj"]["inner"] = "inside";
    
    // We don't need to specify the type of the json object:
    obj["new"]["some"]["deep"]["key"] = "value";
    obj["array2"].append(false, "three");
    
    // We can also parse a string into a json object:
    obj["parsed"] = json::load(R"([{"key": "value"}, false])");
    
    std::cout << obj << '\n';
    return 0;
}
```
Output:
```json
{
    "array" : [true, "two", 3, 4.000000],
    "array2" : [false, "three"],
    "new" : {
        "some" : {
            "deep" : {
                "key" : "value"
            }
        }
    },
    "obj" : {
        "inner" : "inside"
    },
    "parsed" : [{
        "key" : "value"
    }, false]
}
```

This example can also be written another way:
```cpp
#include "json.h"

#include <iostream>

using namespace wingmann; 

int main() {
    json obj {
        "array", json::array(true, "two", 3, 4.0),
        "obj", {
            "inner", "inside"
        },
        "new", { 
            "some", { 
                "deep", { 
                    "key", "value" 
                } 
            } 
        },
        "array2", json::array(false, "three")
    };
    
    std::cout << obj << '\n';
    return 0;
}
```
### Additionally
We do not have access to the colon (:) character in C++, so we cannot use that to seperate key-value pairs,
but by using commas, we can achieve a similar effect.
The other point you might notice, is that we have to explicitly create arrays.
This is a limitation of C++'s operator overloading rules,
so we cannot use the [] operator to define the array.
