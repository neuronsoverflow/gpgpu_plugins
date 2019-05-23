GPGPU Plugins
=========

Description
----

For this project I wanted to created a generic application that would be able to run programs on the GPU.
I created a main executable (`gpgpu`) which is able to load plugins, set their parameters and run their programs. Each plugin (dynamic shared object) has a specific function format (see [pluginHeader.h](src/plugins/include/pluginHeader.h)) which must be used in order for the plugin engine to load the shared libraries appropriately.

Note that the plugins I've written can be run as standalone executables as well. Just use `make executables` in the plugins/ directory to generate them.


Requirements / Libaries
----

  - Linux x86_64 (I've implemented this using Ubuntu 13.10)
  - A GPU device that [suports CUDA](https://developer.nvidia.com/cuda-gpus) (Compute Capability 2+)
  - [CUDA 6.0](https://developer.nvidia.com/cuda-zone) or above


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

Installation
--------------
Assuming that you have the requirements (CUDA6) installed and working, and that you are using a Linux system, then the following should just work.

```sh
# get a copy of the source code
git clone https://github.com/paulohefagundes/gpgpu_plugins.git

# go to the src/ directory
cd gpgpu_plugins/src

# make it so!
make

# run tests
cd ..
./tests/run_tests.rb
```
If there are any errors, check the Makefiles. (I have some hardcoded filepaths such as for CUDA's `nvcc`)

Usage
-------
See [gpgpu readme](src/README.md)


License
----

[MIT](LICENSE.md)
