#pragma once

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define RED "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[39m"
//Combined https://stackoverflow.com/questions/5867834/assert-with-message plus that Nasa 10 commandments code c_assert code snippet.
#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_error(M, ...)\
    fprintf(stderr, "%s [ERROR] %s (%s:%d: errno: %s) " M "\n",\
    RED, RESET, __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define warn(M, ...)\
    fprintf(stdout, "%s[WARNING]%s (%s:%d: errno: %s) " M "\n",\
    YELLOW, RESET, __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define c_assert(e, M, ...)\
    ((e) ? (true) : \
    (log_error(M, ##__VA_ARGS__), false))

#define c_warn(e, M, ...)\
    ((e) ? (true) : \
    (warn(M, ##__VA_ARGS__), false))

