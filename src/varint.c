#include "../include/varint.h"
#include "../include/assertlib.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SEGMENT_BITS 0x7F
#define CONTINUE_BIT 0x80

Varint* varint_vint_from_int(int n) {
    if(!c_assert(n >= 0, "Passed negative integer %d\n", n)){
        return NULL;
    }
    Varint* v = (Varint*)malloc(sizeof(Varint));
    memset(v->data, 0, MAX_VARINT_LEN);

    int i = 0;
    while (n > 127) {
        v->data[i] = (n & 0x7F) | 0x80;  // Store the 7 least significant bits, set the continuation bit
        n >>= 7;  // Right shift by 7 bits
        i++;
    }
    v->data[i] = n & 0x7F;  // Store the final 7 bits, no continuation bit
    v->len = i + 1;  // Total number of bytes written
    return v;
}


int varint_int_from_bytes(const uint8_t* data) {
    if(!c_warn(data != NULL, "passed NULL data pointer.")) {
        return -1;
    }

    uint32_t output = 0;
    int i = 0;
    do {
        output = output | ((data[i] & 0x7F) << (7 * i));
    } while(i < MAX_VARINT_LEN && ((data[i++] & 0x80) != 0));

    return output;
}


int varint_int_from_vint(const Varint* v) {
    if(!c_warn(v != NULL, "passed NULL varint.")) {
        return -1;
    }
    return varint_int_from_bytes(v->data);
}

void varint_free(Varint* v) {
    if(!c_assert(v != NULL, "freeing NULL varint")) {
        return;
    }
    free(v);
}


Varint* varint_vint_from_stream(int fd) {
    errno = 0; 
    if(!c_assert(fd >= 0, "Passed fd %d\n", fd)) {
        return NULL;
    }

    uint8_t buffer[MAX_VARINT_LEN];
    int bytes_read = 0;
    int index = 0;
    bool loop;

    do {
        bytes_read = read(fd, buffer + index, 1);
        loop =  errno == 0 && index < MAX_VARINT_LEN && bytes_read > 0;
        loop &= (buffer[index] & 0x80) == 0x80;
        ++index;
    } while(loop); 

    bool success =  c_warn(errno == 0, "Error reading fd "); 
    success &= c_warn(bytes_read == 1, 
            "Reached end of stream before end of vint (bytes read: %d)",
            bytes_read
            );

    errno = 0;
    if(!success) {
        return NULL;
    }

    Varint* vint = malloc(sizeof(Varint));
    memcpy(vint->data, buffer, index);
    vint->len = index;

    return vint;
}

Varint* varint_vint_from_bytes(const uint8_t* buff) {
    bool precond_met = c_assert(buff != NULL, "Passed NULL buffer");
    if(!precond_met) {
        return NULL;
    }

    Varint* result = malloc(sizeof(Varint));
    int i = 0;
    for(; i < MAX_VARINT_LEN; i++) {
       result->data[i] = buff[i];
       if((buff[i] & CONTINUE_BIT ) == 0) break; 
    }
    result->len = i + 1;
    return result;
}
