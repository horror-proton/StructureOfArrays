# TupleSoA

A toy *structure of arrays* class in C++, assuming types of elements are known at compile time.
`tuple` used here instead of actual `struct` for convenience, so it is called `tuple_of_arrays` for now.

`tuple_of_arrays<A, B, C>` is just roughly a tuple holding arrays of the same size `tuple<vector<A>, vector<B>, vector<C>>`.

~~devil is in the namespace detail~~

## Alternatives
You may also want to try
- [Lunarsong/StructureOfArrays](https://github.com/Lunarsong/StructureOfArrays)
