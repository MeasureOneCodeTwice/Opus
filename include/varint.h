#include <stdint.h>
#include <sys/types.h>
#define MAX_VARINT_LEN 5
typedef struct varint {
    size_t  len;
    uint8_t data[MAX_VARINT_LEN];
} Varint;

Varint* varint_int_to_vint(int n);
int     varint_vint_to_int(Varint* v);
int     varint_bytes_to_int(uint8_t* v);
void    varint_free(Varint* v);
Varint* varint_vint_from_stream(int fd);
