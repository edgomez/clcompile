# CLCompile

- [CLCompile](#clcompile)
  - [Description](#description)
  - [Build](#build)
    - [Requirements](#requirements)
    - [Instructions](#instructions)
  - [Usage](#usage)

## Description

This is a very basic CL program compiler

## Build

### Requirements

- CMake >= 3.10
- C++11 compiler
- OpenCL >= 1.0

### Instructions

This tool is packaged as a very typical C++ CMake based build source tree.

```bash
cmake -B build -S .
cmake --build build -j
```

One can still change usual settings such as:

- the installation prefix with eg: `-DCMAKE_INSTALL_PREFIX=/your/installation/directory/`,
- or the build type with eg: `-DCMAKE_BUILD_TYPE=Release`,
- or the CMake generator with eg: `-G Ninja`

## Usage

```bash
usage: clcompile [OPTION...] <filename...> -- [CLOPTION...]

OPTIONS

-p, --platform-id <INTEGER> Index of the platform to target
-d, --device-id   <INTEGER> Index of the device to target

-h, --help                  Print this help message
-v, --version               Print the program's version

CLOPTIONS

See options listed on https://man.opencl.org/clBuildProgram.html
```
