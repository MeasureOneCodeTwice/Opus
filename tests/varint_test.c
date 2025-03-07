#include "../varint.h"
#include "testlib.h"
#include "../assertlib.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

const int INT_MAX =  2147483647;
const int INT_MIN = -2147483648;

int main(void) {
    int tests_failed = 0;
    int tests_passed = 0;


    //all varints under 128 should 
    //be equal to the number it's int
    //representation.
    Varint* vint;
    bool correct_len = true;
    bool equal = true;
    for(int i = 0; i < 128 && equal; i++) {
        vint = varint_int_to_vint(i);
        equal &= vint->data[0] == i;
        correct_len &= vint->len == 1;
        varint_free(vint);
    }
    verify(equal, "Varints under 128");
    verify(correct_len, "The length of varints under 128");

    //the thresholds for needing another byte.
    for(int i = 1; i < 5; i++) {
        vint = varint_int_to_vint(0x01 << (i * 7));
        verify(vint->data[i] == 0x01, "Randomish varint");
        verify(vint->len == (size_t)(i + 1), "Randomish varint length");
        varint_free(vint);
    }


    vint = varint_int_to_vint(INT_MAX);
    bool as_expected = true;
    for(int i = 0; i < 4; i++) {
        as_expected &= vint->data[i] == 0xFF;
    }
    as_expected |= vint->data[4] == 0x07;
    verify(as_expected, "Max int");
    varint_free(vint);

    uint8_t data[5] = {0x80, 0x01, 0, 0, 0};
    Varint stack_vint = {
        .len = 2,
    };
    memcpy(stack_vint.data, data, 2);
    verify(varint_vint_to_int(&stack_vint) == 128, "Varint (128) to int");

    return summary(&tests_passed, &tests_failed);
}
