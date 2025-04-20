#include "../include/packet.h"
#include "../include/varint.h"
#include "../include/assertlib.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>


#define PACKET_READABLE 0
#define PACKET_WRITABLE 1

#define min(a, b) a < b ? a : b

#define ERRNO_BAD_VAL 22

typedef struct packet_metadata {
    uint8_t packet_type;
    bool    data_copied;
} packetMetadata;

/* typedef struct packet_readable { */
/*     packetMetadata* metadata; */ 
/*     int             packet_id; */   
/*     uint8_t*        data; */
/*     size_t          data_len; */
/* } packetReadable; */

typedef struct packet_writable{
    packetMetadata* metadata;  //metadata for struct, not packet.
    Varint*         packet_len; //len packet_id + len data
    Varint*         packet_id;   //protocol id
    uint8_t*        data;
} packetWritable;



//private functions
int read_all(int sock, void* buff, size_t buff_len); 
void packet_readable_free(packetReadable* p);
void packet_writable_free(packetWritable* p);
bool packet_readable_metadata_eq(
    const packetMetadata* m1, const packetMetadata* m2);
size_t packet_writable_get_data_len(const packetWritable* p);
size_t packet_writable_get_total_len(const packetWritable* p);
void packet_lenDataPair_array_free_warnoption(lenDataPair** pairs, int num_pairs, bool warn);


//-------------------------
//----- PRIVATE UTILS -----
//-------------------------
//
void packet_lenDataPair_free(lenDataPair* pair) {
    bool precond_met = c_warn(pair != NULL, "Passed NULL pair");
    if(!precond_met) {
        return;
    }

    if(pair->data != NULL) {
        free(pair->data);
    }
    free(pair);
}

void packet_lenDataPair_array_free_warnoption(lenDataPair** pairs, int num_pairs, bool warn) {
    bool precond_met = pairs != NULL;
    if(warn) { c_warn(pairs != NULL, "Passed NULL pairs"); }
    if(!precond_met) { return; }

    for(int i = 0; i < num_pairs; i++) {
        bool is_null = pairs[i] == NULL;
        if(warn) { 
            c_warn(pairs[i] != NULL, "pairs contains NULL pair");
        }
        if(!is_null) {
            packet_lenDataPair_free(pairs[i]);
            pairs[i] = NULL;
        }
    }

    if(warn) {
        c_warn(num_pairs > 0, "Passed num_pairs = %d", num_pairs);
    }
    free(pairs);
}

int read_all(int sock, void* buff, size_t buff_len) {

    size_t total_bytes_read = 0;
    int bytes_read;
    bool failure = false;

    do {
        bytes_read = read(sock, buff + total_bytes_read, buff_len - total_bytes_read);
        total_bytes_read += bytes_read;
        failure = bytes_read < 0 || errno != 0;
    } while(!failure && total_bytes_read < buff_len);

    return !failure && total_bytes_read == buff_len ? total_bytes_read : bytes_read;
}


size_t packet_writable_get_data_len(const packetWritable* p) {
    return varint_int_from_vint(p->packet_len) - p->packet_id->len;
}


size_t packet_writable_get_total_len(const packetWritable* p) {
    return varint_int_from_vint(p->packet_len) + p->packet_len->len;
}


void packet_readable_free(packetReadable* p) {
    if (p == NULL) return;

    if(p->metadata->data_copied && p->data != NULL){
        free(p->data);
    }
    free(p->metadata);
    free(p);
}


void packet_writable_free(packetWritable* p) {
    errno = 0;
    if (p == NULL) return;

    if(p->metadata->data_copied && p->data != NULL){
        free(p->data);
    }

    varint_free(p->packet_len);
    varint_free(p->packet_id);
    free(p->metadata);
    free(p);
}


//--------------------------------------------
//----- PUBLIC PACKET READABLE FUNCTIONS -----
//--------------------------------------------

packetReadable* packet_readable_create(
        int packet_id,
        uint8_t* data,
        size_t data_len,
        bool copy_data
) {
    errno = 0;
    bool pre_cond1 = data_len >= 0;
    bool pre_cond2 = data != NULL || data_len == 0;
    c_assert(pre_cond1, "Passed data len %zu (< 0)", data_len);
    c_assert(pre_cond2, "Passed NULL buffer with length %zu", data_len);
    if(!pre_cond1 || !pre_cond2) {
        return NULL;
    }
    
    packetMetadata* metadata = malloc(sizeof(packetMetadata));
    *metadata = (packetMetadata){
        .packet_type = PACKET_READABLE,
        .data_copied = copy_data 
    };

    packetReadable* result = malloc(sizeof(packetReadable));
    *result = (packetReadable){
        .metadata  = metadata,
        .packet_id = packet_id,
        .data_len  = data_len
    };

    if(copy_data) {
        uint8_t* data_copy = malloc(data_len);
        memcpy(data_copy, data, data_len);
        result->data = data_copy;
    } else {
        result->data = data;
    }

    return result;
}


packetReadable* packet_readable_from_writable(
        packetWritable* p, bool copy_data
) {
    errno = 0;
    if(!c_warn(p != NULL, "passed NULL packet.\n")){
        return NULL;
    }
    
    packetMetadata* metadata = malloc(sizeof(packetMetadata));
    *metadata = (packetMetadata){
        .packet_type = PACKET_READABLE,
        .data_copied = copy_data,
    };

    int packet_id = varint_int_from_vint(p->packet_id);
    
    size_t data_len = 
        varint_int_from_vint(p->packet_len) - p->packet_id->len;

    packetReadable* p_readable = malloc(sizeof(packetReadable));
    *p_readable = (packetReadable){
        .metadata  = metadata,
        .packet_id = packet_id,
        .data_len  = data_len
    };

    if(copy_data) {
        uint8_t* data_copy = malloc(data_len);
        memcpy(data_copy, p->data, data_len);
        p_readable->data = data_copy;
    } else {
        p_readable->data = p->data;
    }
    

    return p_readable;
}


//does not compare the copied field.
bool packet_readable_eq(
        const packetReadable* p1,
        const packetReadable* p2
) {
    bool precond_met = !c_warn(p1 != NULL, 
            "passing p1 == NULL\n."); 
    precond_met &= !c_warn(p1 != NULL, 
            "passing p2 == NULL\n.");

    if(!precond_met) {
        return false;
    }

    bool eq = 
    p1->metadata->packet_type == p2->metadata->packet_type;
    eq &= p1->packet_id == p2->packet_id;
    eq &= p1->data_len  == p2->data_len;
    if(eq) {
        eq &= memcmp(p1->data, p2->data, p2->data_len) == 0;
    }
    return eq;
}


//--------------------------------------------
//----- PUBLIC PACKET WRITABLE FUNCTIONS -----
//--------------------------------------------

packetWritable* packet_writable_from_readable(
        packetReadable* p, bool copy_data 
) {
    errno = 0;

    if(!c_warn(p != NULL, "passed NULL packet.\n")) {
        return NULL;
    }

    packetMetadata* metadata = malloc(sizeof(packetMetadata));
    *metadata = (packetMetadata){
        .packet_type = PACKET_WRITABLE, 
        .data_copied = copy_data
    };

    packetWritable* p_writable = malloc(sizeof(packetWritable));
    *p_writable = (packetWritable){
        .metadata   = metadata,
        .packet_id  = varint_vint_from_int(p->packet_id),
    };

    if(copy_data) {
        uint8_t *data_copy = malloc(p->data_len);
        memcpy(data_copy, p->data, p->data_len);
        p_writable->data = data_copy;
    } else {
        p_writable->data = p->data;
    }

    p_writable->packet_len =
        varint_vint_from_int(p->data_len + p_writable->packet_id->len);

    return p_writable;
}


bool packet_stream_from_writable(int fd, const packetWritable* p) {

    errno = 0;
    bool precond_met = c_warn(fd >= 0, "passed fd = %d\n", fd);
    precond_met &= c_warn(p != NULL, "passed NULL packet\n");
    if(!precond_met) { 
        return false;
    }

    errno = 0;
    size_t packet_buffer_len = packet_writable_get_total_len(p);
    uint8_t packet_buffer[packet_buffer_len];

    size_t data_len = packet_writable_get_data_len(p);

    //pointer arithmetic
    uint8_t* buffer_ptr = (uint8_t*)packet_buffer;
    memcpy(buffer_ptr, p->packet_len->data, p->packet_len->len);
    buffer_ptr +=  p->packet_len->len;
    memcpy(buffer_ptr, p->packet_id->data,  p->packet_id->len); 
    buffer_ptr += p->packet_id->len;
    memcpy(buffer_ptr, p->data,             data_len); 
    
    int bytes_written = write(fd, packet_buffer, packet_buffer_len);

    bool is_write_success = c_assert(errno == 0,
            "Error writing to given file descriptor "
            "in packet_writable_write"
    );
    
    is_write_success &= c_assert(
            bytes_written == (int)packet_buffer_len, 
            "Partial write of packet in packet_writable_write. "
            "%d/%zu bytes written",
            bytes_written,
            packet_buffer_len);

    errno = 0;

    return is_write_success;
}


packetWritable* packet_writable_from_stream(int fd) {

    errno = 0;
    if(!c_assert(fd >= 0, "Passed fd %d", fd)) {
        return NULL;
    }

    Varint* packet_len_vint = varint_vint_from_stream(fd);
    bool error = !c_assert( packet_len_vint != NULL, 
            "Failed reading packet length from stream."
    );
    if(error) { return NULL; }

    int packet_len = varint_int_from_vint(packet_len_vint);

    Varint* packet_id_vint = varint_vint_from_stream(fd);
    error = !c_assert(packet_id_vint != NULL, 
            "Failed to read packet_id vint from stream."
    );
    if(error) {
        free(packet_len_vint);
        return NULL;
    }

    errno = 0;
    size_t data_len = packet_len - packet_id_vint->len;
    uint8_t* data = NULL;
    if(data_len != 0) {
        data = malloc(data_len);

        int bytes_read = read_all(fd, data, data_len);
        error = !c_assert(errno == 0,
                "Error reading packet body."
        );
        error &= !c_assert(bytes_read == (int)data_len,
                "Packet data length: %zu, bytes read: %d\n",
                data_len,
                bytes_read
        ); 

        if(error) {
            errno = 0;
            varint_free(packet_len_vint);
            varint_free(packet_id_vint);
            free(data);
            return NULL;
        }
    }

    packetMetadata* metadata = malloc(sizeof(packetMetadata));
    metadata->data_copied = true;
    metadata->packet_type = PACKET_WRITABLE;

    packetWritable* p = malloc(sizeof(packetWritable));
    p->metadata = metadata;
    p->packet_len = packet_len_vint;
    p->packet_id  = packet_id_vint;
    p->data = data;

    return p;
}


//does not compare the copied field.
bool packet_writable_eq(
        const packetWritable* p1,
        const packetWritable* p2
){
    errno = 0;
    bool precond_met = 
    c_warn(p1 != NULL, "passing p1 == NULL."); 
    precond_met &= c_warn(p1 != NULL, "passing p2 == NULL.");

    if(!precond_met) {
        return false;
    }


    bool eq = 
        p1->metadata->packet_type == p2->metadata->packet_type;

    eq &= memcmp(
            p1->packet_len->data,
            p2->packet_len->data, 
            min(p1->packet_len->len, p2->packet_len->len)
        ) == 0;

    eq &= memcmp(
            p1->packet_id->data,
            p2->packet_id->data, 
            min(p1->packet_id->len, p2->packet_id->len)
        ) == 0;

    size_t p1_data_size = packet_writable_get_data_len(p1);
    size_t p2_data_size = packet_writable_get_data_len(p2);
    eq &= p1_data_size == p2_data_size;

    if(eq) {
        eq &= memcmp(p1->data, p2->data, p1_data_size) == 0;
    }

    return eq;

}


size_t packet_writable_length(const packetWritable* p) {
    if(!c_assert(p != NULL, "")) { 
        errno = ERRNO_BAD_VAL;
        return 0; 
    }
    return varint_int_from_vint(p->packet_len) +
        p->packet_len->len;
}

//--------------------------------------
//----- PACKET BODY PUBLIC FUNCTIONS ---
//--------------------------------------

lenDataPair** packet_lenDataPair_array_from_packetReadable_body(
        const packetReadable* packet,
        int num_pairs
) {
    bool precond_met = 
       c_assert(packet != NULL, "Passed NULL body")
    && c_assert(packet->data != NULL, "Packet has NULL data field")
    && c_warn(packet->data_len > 0 && num_pairs > 0, 
            "body_len: %zu num_pairs: %d", 
             packet->data_len, num_pairs
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
        packet_lenDataPair_array_free_warnoption(result, num_pairs, false);
        return NULL; 
    }


    //parse data into the pairs
    size_t offset = 0;
    i = 0;
    for(; i < num_pairs && offset <= packet->data_len; i++) {

        Varint* chunk_body_len_vint = varint_vint_from_bytes(packet->data + offset);
        failure = !c_assert(
            chunk_body_len_vint != NULL,
            "Could not parse beginning of chunk to varint"
        );
        if(failure) break;
        size_t chunk_body_len = (size_t)varint_int_from_vint(chunk_body_len_vint);
        offset += chunk_body_len_vint->len; 
        varint_free(chunk_body_len_vint);
        failure = !c_assert(
            offset + chunk_body_len <= packet->data_len,
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
        memcpy(result[i]->data, packet->data + offset, chunk_body_len);
        offset += chunk_body_len;
    }
    //assert the chunks spanned the entire buffer and there is nothing left over.
    failure |= !c_assert( !(i == num_pairs && offset != packet->data_len), 
        "Actual number of chunks exceeds number specified (%zu bytes remaining)",
        packet->data_len - offset
    );

    if(failure) { packet_lenDataPair_array_free_warnoption(result, num_pairs, false); }
    return failure ? NULL : result;
}

//----------------------------------
//----- FREEING PUBLIC FUNCTIONS -----
//----------------------------------

//a little bit of polymorphism :)
void packet_free(Packet *p) {
    errno = 0;
    if(!c_warn(p != NULL, "passing NULL to packet_free")) {
        return;
    }

    switch(p->metadata->packet_type) {
        case PACKET_READABLE:
            packet_readable_free((packetReadable*)p);
            break;
        case PACKET_WRITABLE:
            packet_writable_free((packetWritable*)p);
            break;
        default:
            c_assert(false, "(packet_free):"
                    "packet type matched no known packet type");
            break;
    }

}

void packet_lenDataPair_array_free(lenDataPair** pairs, int num_pairs) {
    packet_lenDataPair_array_free_warnoption(pairs, num_pairs, false);
}


