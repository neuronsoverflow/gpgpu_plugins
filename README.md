GPGPU Plugins
=========

Description
----

For this project I wanted to created a generic application that would be able to run programs on the GPU.
I created a main executable (`gpgpu`) which is able to load plugins, set their parameters and run their programs. Each plugin (dynamic shared object) has a specific function format (see [pluginHeader.h](src/plugins/include/pluginHeader.h)) which must be used in order for the plugin engine to load the shared libraries appropriately.

Note that the plugins I've written can be run as standalone executables as well. Just use `make executables` in the plugins/ directory to generate them.

Requirements
-----
  - Linux x86_64 (I've originally implemented this using Ubuntu 13.10)
  - A GPU device that [supports CUDA](https://developer.nvidia.com/cuda-gpus) (Compute Capability 2+)
  - [CUDA 6.0](https://developer.nvidia.com/cuda-zone) or above (I've recently tested this on CUDA 10 and it still works)
  - CMake 3.13 or newer

Plugins
-----------
The following are plugins that have been written.
- `matrix.so` - Matrix Multiplication ~ a simple matrix multiplication
- `prime.so` - Uses the Sieve of Eratosthenes for generating Prime Numbers
- `graph.so` - Graph Search ~ loads a graph and allows the user to run these algorithms:
    - Breadth-First Search
    - Single-Source Shortest Path
    - All-Pairs Shortest Path

All of the graph algorithms were based on the algorithms given by Pawan Harish and P.J. Narayanan (HiPC 2007)

Build
--------------
Assuming that you have the requirements (CUDA 6 or newer and CMake) installed and working, and that you are using a Linux system, then the following should just work.

```sh
# get a copy of the source code
git clone https://github.com/paulohefagundes/gpgpu_plugins.git

# build
mkdir -p build
cd build
cmake -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$(pwd)/bin" -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$(pwd)/lib" ..
cmake --build .

# run tests
cd ..
./tests/run_tests.rb
```

Usage
-------
See [gpgpu readme](src/main/README.md)


License
----

[MIT](LICENSE.md)
