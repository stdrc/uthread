#pragma once

#ifdef DEBUG
#define debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif

#define fatal(exit_code, fmt, ...)  \
    do {                            \
        printf(fmt, ##__VA_ARGS__); \
        exit(exit_code);            \
    } while (0)
