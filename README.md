# Structure of Arrays

A toy *structure of arrays* class in C++ 17, assuming types of elements are known at compile time.

Currently here are two implementations, the meta-struct version and `std::tuple` version.

## Usage

```c++
using vec4 = std::array<float, 4>;

structure_of_arrays<vec4, vec4, float> particles;

particles.reserve(...);

particles.push_back(vec4{0.F, 0.F, 0.F, 0.F}, vec4{0.F, 0.F, 1.F, 0.F}, 2.F);
particles.push_back(...);
particles.push_back(...);

std::cout << particles.get<2>(0) << std::endl; // 2.0

particles.pop_back();

particles.insert(index, vec4{}, vec4{}, float{});
```

## Alternatives
- [Lunarsong/StructureOfArrays](https://github.com/Lunarsong/StructureOfArrays)
