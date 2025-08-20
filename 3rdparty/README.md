# Tiny3D third-party libraries

This folder contains a set of external libraries that are used in Tiny3D. Tiny3D
prefers building third-party dependencies from source to detecting pre-installed
system dependencies.

## List of third-party libraries

```txt

--------------------------------------------------------------------------------
Eigen                       3.4                              Mainly MPL2 license
A high-level C++ library of template headers for linear algebra, matrix and
vector operations, numerical solvers and related algorithms
http://eigen.tuxfamily.org/
--------------------------------------------------------------------------------
flann                       1.8.4                                    BSD license
A C++ library for performing fast approximate nearest neighbor searches in high
dimensional spaces
http://www.cs.ubc.ca/research/flann/
--------------------------------------------------------------------------------
jsoncpp                     1.8.4                                    MIT license
A C++ library that allows manipulating JSON values
https://github.com/open-source-parsers/jsoncpp
--------------------------------------------------------------------------------
nanoflann                   1.3.1                                    BSD license
A C++11 header-only library for Nearest Neighbor (NN) search with KD-trees
https://github.com/jlblancoc/nanoflann
--------------------------------------------------------------------------------
pybind11                    v2.13.1                                  BSD license
Python binding for C++11
https://github.com/pybind/pybind11
--------------------------------------------------------------------------------
RPly                        1.1.3                                    MIT license
A library to read and write PLY files
http://w3.impa.br/~diego/software/rply/
--------------------------------------------------------------------------------
```

Finally, this patch can be used in CMake `ExternalProject_Add` by specifying:

```cmake
find_package(Git QUIET REQUIRED)

ExternalProject_Add(
    ...
    PATCH_COMMAND ${GIT_EXECUTABLE} init
    COMMAND       ${GIT_EXECUTABLE} apply --ignore-space-change --ignore-whitespace
                  /path/to/0001-Patch-Assimp-Obj-importer.patch
    ...
)
```
