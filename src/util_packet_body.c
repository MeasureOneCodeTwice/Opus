#include "../include/util_packet_body.h"
#include "../include/varint.h"
#include "../include/assertlib.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//Private util
void util_packet_body_lenDataPair_pairs_free_warnoption(lenDataPair** pairs, int num_pairs, bool warn);

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
    if(!precond_met) { return NULL; }


    //malloc array of lenDataPairs
    lenDataPair** result = malloc(sizeof(lenDataPair*) * num_pairs); 
    int i = 0;
    for(; i < num_pairs && result != NULL; i++) {
        result[i] = malloc(sizeof(lenDataPair));
        if(result[i] == NULL) { break; }
        result[i]->len  = 0;
        result[i]->data = NULL;
    }
    bool failure = false;
    failure = !c_assert(
            result != NULL, 
            "Failed to allocate space for array of lenDataPair pointers"
    );
    failure = failure || !c_assert(
            !failure && i == num_pairs,
            "Failed to allocate space for lenDataPairs"
    );
    if(failure) {
        util_packet_body_lenDataPair_pairs_free_warnoption(result, num_pairs, false);
        return NULL; 
    }


    //parse data into the pairs
    size_t offset = 0;
    i = 0;
    for(; i < num_pairs && offset <= body_len; i++) {

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
    //assert the chunks spanned the entire buffer and there is nothing left over.
    failure |= !c_assert( !(i == num_pairs && offset != body_len), 
        "Actual number of chunks exceeds number specified (%zu bytes remaining)",
        body_len - offset
    );

    if(failure) { util_packet_body_lenDataPair_pairs_free_warnoption(result, num_pairs, false); }
    return failure ? NULL : result;
}

void util_packet_body_lenDataPair_pairs_free_warnoption(lenDataPair** pairs, int num_pairs, bool warn) {
    bool precond_met; 
    if(warn) {
        precond_met = c_warn(pairs != NULL, "Passed NULL pairs");
    } else { 
        precond_met = pairs != NULL;
    }
    if(!precond_met) { return; }

    for(int i = 0; i < num_pairs; i++) {
        bool is_null;
        if(warn) {
            is_null = !c_warn(pairs[i] != NULL, "pairs contains NULL pair");
        } else {
            is_null = pairs[i] == NULL;
        }
        if(!is_null) {
            util_packet_body_lenDataPair_free(pairs[i]);
        }
    }

    if(warn) {
        c_warn(num_pairs > 0, "Passed num_pairs = %d", num_pairs);
    }
    free(pairs);

}

void util_packet_body_lenDataPair_pairs_free(lenDataPair** pairs, int num_pairs) {
    util_packet_body_lenDataPair_pairs_free_warnoption(pairs, num_pairs, true);
}


void util_packet_body_lenDataPair_free(lenDataPair* pair) {
    bool precond_met = c_warn(pair != NULL, "Passed NULL pair");
    if(!precond_met) {
        return;
    }

    if(pair->data != NULL) {
        free(pair->data);
    }
    free(pair);
}
