#pragma once

#include <stdint.h>
#include <sys/types.h>

typedef struct len_data_pair {
    size_t len;
    uint8_t* data;
} lenDataPair;

lenDataPair** util_packet_parse_generic_body(
        const uint8_t* body,
        size_t body_len,
        int num_pairs
);


void util_packet_body_lenDataPair_pairs_free(
        lenDataPair** pairs,
        int num_pairs
); 


void util_packet_body_lenDataPair_free(lenDataPair* pair); 
