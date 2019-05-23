cmake_minimum_required(VERSION 3.13)

project(gpgpu-plugins LANGUAGES CXX C CUDA)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(clang-format)
include(clang-tidy)
include(compiler-warnings)

add_subdirectory(src)
