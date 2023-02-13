cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(CLCompile
  LANGUAGES CXX
)

find_package(OpenCL REQUIRED)

add_executable(clcompile
  src/clcompile.cpp
  src/clutils.cpp
  src/clutils.h
  src/scope_guard.h
)

target_compile_definitions(clcompile
  PRIVATE
    CL_TARGET_OPENCL_VERSION=100
)

target_link_libraries(clcompile
  PUBLIC
    OpenCL::OpenCL
)

target_compile_features(clcompile
  PUBLIC
    cxx_std_11
)