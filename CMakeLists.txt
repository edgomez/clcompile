cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(CLCompile
  LANGUAGES CXX
)

find_package(OpenCL REQUIRED)

add_executable(clcompile
  src/main.cpp
  src/clc.cpp
  src/clc.h
  src/log.h
  src/scope_guard.h
)

math(EXPR CL_TARGET_OPENCL_VERSION
  "${OpenCL_VERSION_MAJOR} * 100 + ${OpenCL_VERSION_MINOR}*10"
  OUTPUT_FORMAT
    DECIMAL
)

target_compile_definitions(clcompile
  PRIVATE
    CL_TARGET_OPENCL_VERSION=${CL_TARGET_OPENCL_VERSION}
)

target_link_libraries(clcompile
  PUBLIC
    OpenCL::OpenCL
)

target_compile_features(clcompile
  PUBLIC
    cxx_std_11
)