[![build][build img]][build]
[![codeql][codeql img]][codeql]

# libcstl
A collection of common data structures, not unlike the C++ STL, for C

## Documentation
Documentation of the library and its interfaces can be found
[here](https://johntyner.github.io/libcstl).

The library contains several data structures that do not require
memory allocation at the expense of requiring the caller to modify their
data structures. Over time, these "low-level" interfaces will be given
corresponding higher level interfaces that allocate memory but do not require
callers to modify their own data structures.

## Some history
I have implemented the red-black tree found in this repository several
times over the years, but I've never saved it either because I implemented
it for work or lost the implementation when changing computers. This
library started out as an attempt to implement it in a way that wouldn't
get lost but quickly grew into implementations of several data structures
I've repeatedly implemented over the years.

In addition to those, I've also always said that I really like C, but it
needs something better to deal with strings. This library contains
something that approximates the C++ STL version of a string to address that.

[build]: https://github.com/johntyner/libcstl/actions/workflows/c-cpp.yml
[build img]: https://github.com/johntyner/libcstl/actions/workflows/c-cpp.yml/badge.svg
[codeql]: https://github.com/johntyner/libcstl/actions/workflows/codeql.yml
[codeql img]: https://github.com/johntyner/libcstl/actions/workflows/codeql.yml/badge.svg