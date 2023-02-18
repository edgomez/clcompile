// SPDX-License-Identifier: MIT
// Copyright 2023 Edouard Gomez

#ifndef log_h
#define log_h

#include <cstdio>

#define logerr(...)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        std::fprintf(stderr, "error: " __VA_ARGS__);                                                                   \
    } while (0)

#define loginfo(...)                                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        std::fprintf(stdout, "info: " __VA_ARGS__);                                                                    \
    } while (0)

#endif // log_h
