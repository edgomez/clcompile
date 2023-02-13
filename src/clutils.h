// SPDX-License-Identifier: MIT
// Copyright 2023 Edouard Gomez

#ifndef clutils_h
#define clutils_h

#include <CL/cl.h>

namespace clc
{

const char *cl_error_str(cl_int errorcode);

} // namespace clc

#endif // clutils_h