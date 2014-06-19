GPGPU Plugins
=========

Description
----

This project is NOT yet finished!
I'm currently coding it such that it will:
- Allow the user to load plugins into it, and they will run on the GPU


Requirements / Libaries
----

  - Linux x86_64
  - A GPU that [suports CUDA](https://developer.nvidia.com/cuda-gpus) or [OpenCL 1.1+](http://www.khronos.org/conformance/adopters/conformant-products#opencl)
  - [OpenCL 1.1+](https://www.khronos.org/opencl/)
  - [CUDA 6.0](https://developer.nvidia.com/cuda-zone) (still deciding if this will be required)


Plugins
-----------
The following are plugins that are planned for being written.
- Matrix Multiplication ~ a simple matrix multiplication
- Prime Number Generator ~ uses the parallel computing power of GPUs to generate prime numbers
- Graph Search ~ loads a graph and allows the user to run these algorithms:
    - Breadth-First Search
    - Single-Source Shortest Path
    - All-Pairs Shortest Path


Installation
--------------
After having
```sh
git clone git@github.com:paulohefagundes/gpgpu_plugins.git
cd gpgpu_plugins/src
make
```

Usage
-------
to be written....


License
----

MIT