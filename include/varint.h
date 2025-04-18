#pragma once

#include <stdint.h>
#include <sys/types.h>
#define MAX_VARINT_LEN 5
typedef struct varint {
    size_t  len;
    uint8_t data[MAX_VARINT_LEN];
} Varint;

Varint* varint_vint_from_int(int n);
int     varint_int_from_vint(const Varint* v);
int     varint_int_from_bytes(const uint8_t* v);
void    varint_free(Varint* v);
Varint* varint_vint_from_stream(int fd);
Varint* varint_vint_from_bytes(const uint8_t* buff);
