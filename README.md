# Pydex
Compile time pythonic indexing for C++20

<h1> 
Pydexability: 
</h1>

Any type that satisfies this concept
```cpp
template<typename T> concept Pydexable = requires(T x) { x[x.size()]; };
```
is usable with `pydex`, slices returned by `pydex` are also pydexable.

<h1>
Supported:
</h1>

Constant indexing:
```cpp
auto& x = pydex<"1,2,3,-4">(arr); // x = arr[1,2,3,-4]
```

Constant slicing:
```cpp
auto& x = pydex<"1:3, 3, :, -5:">(arr); // x = arr[1:3, 3, :, -5:]
// Prints arr[1, 3, 3, -5:]
std::cout << x[0][3] << std::endl;
```

Steps:
```cpp
auto& x = pydex<"::2">(arr); // Every other element in arr
```

Ellipsis:
```cpp
auto& x = pydex<"..., 3">(arr); // arr[..., 3]
```

Assignment:
```cpp
pydex<"1,2,3,-4">(arr) = 5; // arr[1,2,3,-4] = 5
pydex<"0:3">(arr) = {3,4,5}; // arr[0:3] = [3,4,5]
```
Assigning to a slice from another slice where both slices share the underlying container 
and have overlapping elements is not safe if an element gets overwritten in the LHS slice before it is read in the RHS slice, e.g:
```cpp
std::vector<int> vec{1, 2, 3, 4, 5};
pydex<"1:3">(vec) = pydex<"0:2">(vec); // Unsafe
// should print [1, 1, 2, 4, 5], actually prints [1, 1, 1, 4, 5]
std::cout << pydex<":">(vec) << std::endl; 
```
```cpp
std::vector<int> vec2{1, 2, 3, 4, 5};
pydex<"0:2">(vec2) = pydex<"1:3">(vec2); // Safe
// prints [2, 3, 3, 4, 5]
std::cout << pydex<":">(vec2) << std::endl;
```
`std::ostream` integration:
```cpp
std::cout << pydex<":, 3">(arr) << std::endl; // print(arr[:, 3])
```
Irregular shapes:
```cpp
std::vector<std::vector<int>> data2 {
       {1,2,3},
       {1,2,3,4,5,6,7,8},
       {9,10,11,12,13,14,15,16,17,18,19,20}
};
pydex<":,5">(data2)[0]; // throws std::out_of_range
pydex<":,5">(data2)[1]; // 6
pydex<":,5">(data2)[2]; // 14
pydex<":,5">(data2)[-1]; // 14
pydex<"199,5">(data2); // throws std::out_of_range
pydex<"3, 88:1531">(data2) // Empty
```

Deep copies:
```cpp
std::vector<std::vector<int>> data2{{1, 2,  3},
                                    {1, 2,  3,  4,  5,  6,  7,  8},
                                    {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}};

std::vector<std::vector<int>> arr3 = pydex<":, 1">(data2).copy();
// Prints {{2,2,10}}
std::cout << pydex<"...">(arr3) << '\n';
```
<h1> Unsupported </h1>

Variable slicing:
```python
x = arr[a:b] #unsupported
```

<h1> Bounds checking </h1>
Bounds checking can be disabled manually like so

```cpp
pydex<"...", /*bounds checks*/ false>(...);
```

Disabling it will break some things but keeping it enabled has significant performance overhead