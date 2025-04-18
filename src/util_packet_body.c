#include "../include/util_packet_body.h"
#include "../include/varint.h"
#include "../include/assertlib.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

lenDataPair** util_packet_parse_generic_body(
        const uint8_t* body,
        size_t body_len,
        int num_pairs
) {
    bool precond_met = c_assert(body != NULL, "Passed NULL body");
    precond_met &= c_warn(body_len > 0 && num_pairs > 0, 
            "body_len: %zu num_pairs: %d", 
            body_len, num_pairs
    );
    if(!precond_met) {
        return NULL;
    }

    bool failure = false;
    lenDataPair** result = malloc(sizeof(lenDataPair*) * num_pairs); 
    failure = !c_assert(
            result != NULL, 
            "Could not allocate space for array of lenDataPair pointers"
    ); 

    if(failure) {
        return NULL;
    }

    size_t offset = 0;
    int i = 0;
    for(; i < num_pairs && offset <= body_len && body_len; i++) {

        result[i] = malloc(sizeof(lenDataPair));
        failure = !c_assert(
            result != NULL, 
            "Could not allocate space for lenDataPair"
        ); 
        if(failure) break;

        Varint* chunk_body_len_vint = varint_vint_from_bytes(body + offset);
        failure = !c_assert(
            chunk_body_len_vint != NULL,
            "Could not parse beginning of chunk to varint"
        );
        if(failure) break;

        size_t chunk_body_len = (size_t)varint_int_from_vint(chunk_body_len_vint);
        offset += chunk_body_len_vint->len; 
        varint_free(chunk_body_len_vint);
        failure = !c_assert(
            offset + chunk_body_len <= body_len,
            "Data chunk is larger than the space remaining in body."
        );
        if(failure) break;

        failure = !c_assert(offset + chunk_body_len != body_len || i + 1 == num_pairs, 
            "Number of chunks are less than number specified (%d < %d)",
            i + 1,
            num_pairs
        );
        if(failure) break;

        //copy the chunk to a data len pair
        result[i]->len = chunk_body_len;
        result[i]->data = malloc(chunk_body_len);
        failure = !c_assert(
            result[i]->data != NULL, 
            "Could not allocate chunk body"
        ); 
        if(failure) break;
        memcpy(result[i]->data, body + offset, chunk_body_len);
        offset += chunk_body_len;
    }
    failure |= !failure && !c_assert(offset == body_len, 
        "Number of chunks is more than specificed"
    );

    if(failure) {
        util_packet_body_lenDataPair_pairs_free(result, i);
    }

    return failure ? NULL : result;
}

void util_packet_body_lenDataPair_pairs_free(lenDataPair** pairs, int num_pairs) {
    bool precond_met = c_warn(pairs != NULL, "Passed NULL pairs");
    if(!precond_met) {
        return;
    }

    for(int i = 0; i < num_pairs; i++) {
        bool is_null = !c_warn(pairs[i] != NULL, "pairs contains NULL pair");
        if(!is_null) {
            util_packet_body_lenDataPair_free(pairs[i]);
        }
    }

    c_warn(num_pairs > 0, "Passed num_pairs = %d", num_pairs);
    free(pairs);
}


void util_packet_body_lenDataPair_free(lenDataPair* pair) {
    bool precond_met = c_warn(pair != NULL, "Passed NULL pair");
    if(!precond_met) {
        return;
    }

    //may be legitimately be null on malloc(0)
    if(pair->data != NULL) {
        free(pair->data);
    }
    free(pair);
}
