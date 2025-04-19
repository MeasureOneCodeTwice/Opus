#include "../include/util_packet_body.h"
#include "../include/packet.h"
#include "../include/testlib.h"

#include <stdio.h>

int tests_passed = 0;
int tests_failed = 0;

void test_parse_generic_packet_body(void);

int main(void) {
    test_parse_generic_packet_body();
    summary(&tests_passed, &tests_failed);
    return tests_failed;
}

void test_parse_generic_packet_body(void) {

    uint8_t* content; 
    lenDataPair** result;

    result = util_packet_parse_generic_body(NULL, 0, 0);
    verify(result == NULL, "Can handle NULL buffer");

    content = (uint8_t[3]){ 0x03, 0x00, 0x00};
    result = util_packet_parse_generic_body(content, 3, 1);
    verify(result == NULL, "Varint signifies length larger than actual length");

    content = (uint8_t[3]){ 0x02, 0x00, 0x00};
    result = util_packet_parse_generic_body(content, 3, 10);
    verify(result == NULL, "Specify more pairs than exist");

    content = (uint8_t[5]){ 0x01, 0x00, 0x02, 0x00, 0x00};
    result = util_packet_parse_generic_body(content, 5, 1);
    verify(result == NULL, "Specify less pairs than exist");

    result = util_packet_parse_generic_body(content, 5, 2);
    verify(result != NULL, "Test simple valid case");

    bool expected_contents = false;
    if(result != NULL) {
        expected_contents = true;
        expected_contents &= result[0]->len == 1;
        expected_contents &= result[0]->data[0] == 0x00;
        expected_contents &= result[1]->len == 2;
        expected_contents &= result[1]->data[0] == 0x00;
        expected_contents &= result[1]->data[1] == 0x00;
        util_packet_body_lenDataPair_pairs_free(result, 2);
    }
    verify(expected_contents, "Contents of simple valid case correct");
}

