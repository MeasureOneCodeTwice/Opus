#include "../include/mc_protocol.h"

#include "../include/varint.h"
#include "../include/packet.h"
#include "../include/assertlib.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define MC_PROTOCOL_PROTOCOL_VERSION 769
#define MC_PROTOCOL_MAX_RESPONSE_SIZE 32767
#define MAX_USERNAME_LENGTH 16

#define LOGIN_REQUEST_PACKET_ID 0

typedef struct encryption_request_response_writable {
    Varint*  server_id_len;
    char*    server_id; //len <= 20
    Varint*  public_key_length;
    uint8_t* public_key;
    Varint*  verify_token_length;
    uint8_t* verify_token;
    uint8_t  should_authenticate;
} encryptionRequestResponseWritable;

typedef struct len_data_pair {
    size_t len;
    uint8_t* data;
} lenDataPair;

/* enum SERVER_STATE { */
/*     STATE_STATUS   = 1, */
/*     STATE_LOGIN    = 2, */
/*     STATE_TRANSFER = 3 */
/* }; */

//-------------------------------------
//-----PRIVATE UTIL PREDECLARATION-----
//-------------------------------------

lenDataPair** parse_generic_packet_body(
        const uint8_t* body,
        size_t body_len,
        int num_pairs
);

uint8_t* get_handshake_packet_body(
        int protocol_version,
        const char server_addr[255],
        uint16_t port,
        int next_state,
        size_t* buff_len
);

//-------------------------
//----- PRIVATE UTILS -----
//-------------------------

//handsake requires:
//protocol version (varint)
//server address   (string)   unused by server
//server port      (uint16_t) unused by server
//next stat        (varint) 
uint8_t* get_handshake_packet_body(int protocol_version, const char server_addr[255], uint16_t port, int next_state, size_t* buff_len) {

    size_t server_address_len = strnlen(server_addr, 254) + 1;

    Varint *protocol_version_vint  = varint_int_to_vint(protocol_version);
    Varint *server_addr_len_vint   = varint_int_to_vint(server_address_len);
    Varint *next_state_vint        = varint_int_to_vint(next_state);


    *buff_len = 
        protocol_version_vint->len  +
        server_addr_len_vint->len   + 
        server_address_len          +
        2                           + //length of port
        next_state_vint->len;

    uint8_t* buff = malloc(*buff_len);
    uint8_t* buff_index = buff; //will do pointer arithmitic on this.

    memcpy(buff_index, (void*)protocol_version_vint->data, protocol_version_vint->len);
    buff_index += protocol_version_vint->len;
    memcpy(buff_index, (void*)server_addr_len_vint->data,  server_addr_len_vint->len);
    buff_index += server_addr_len_vint->len;
    memcpy(buff_index, (void*)server_addr,                 server_address_len);
    buff_index  += server_address_len;
    memcpy(buff_index, (void*)&port,                       2);
    buff_index   += 2;
    memcpy(buff_index, (void*)next_state_vint->data,       next_state_vint->len);

    varint_free(protocol_version_vint);
    varint_free(server_addr_len_vint);
    varint_free(next_state_vint);

    return buff;
}


lenDataPair** parse_generic_packet_body(
        const uint8_t* body,
        size_t body_len,
        int num_pairs
) {
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
    for(; i < num_pairs && body_len && !failure; i++) {
        result[i] = malloc(sizeof(lenDataPair));
        failure = !c_assert(
            result != NULL, 
            "Could not allocate space for lenDataPair"
        ); 
        if(failure) break;

        int chunk_len = varint_bytes_to_int(body);
        failure = !c_assert(
            chunk_len >= 0,
            "Could beginning of chunk to varint"
        );
        if(failure) break;
        offset += chunk_len;

        result[i]->len = chunk_len;
        result[i]->data = malloc(chunk_len);
        failure = !c_assert(
            result[i]->data != NULL, 
            "Could not allocate chunk body"
        ); 
        if(failure) break;

        memcpy(result[i]->data, body + offset, chunk_len);
        offset += chunk_len;
    }

    if(failure) {
        //don't go freeing all the way, we need to be careful with 
        //freeing the last bit of data
        for(int j = 0; j < i - 1 ; j++) {
            free(result[j]->data);
            free(result[j]);
        }

        if(result[i - 1] != NULL) {
            if(result[i - 1]->data != NULL) {
                free(result[i - 1]->data);
            }
            free(result[i - 1]);
        }

        free(result);
    }

    return failure ? NULL : result;
}



//----------------------------
//----- PUBLIC FUNCTIONS -----
//----------------------------
//
bool mc_protocol_handshake_custom(
            int sock,  
            const char* server_addr,
            uint16_t server_port,
            enum SERVER_STATE next_state
) {
    size_t packet_body_len = 0;
    uint8_t* handshake_packet_body = get_handshake_packet_body(
            MC_PROTOCOL_PROTOCOL_VERSION,
            server_addr,
            server_port,
            next_state,
            &packet_body_len
        );

    packetReadable* packet_r = packet_readable_create(
        0x00, 
        handshake_packet_body,
        packet_body_len,
        false
        );
    if(!c_assert(packet_r != NULL, "Error creating handshake packet readable.")) {
        return 1;
    }

    packetWritable* packet_w = packet_writable_from_readable(
        packet_r, 
        false
        );
    if(!c_assert(packet_w != NULL, "Error creating handshake packet writable.")) {
        packet_free((Packet*)packet_r);
        free(handshake_packet_body); 
        return 1;
    }

    bool success = packet_stream_from_writable(sock, packet_w); 

    packet_free((Packet*)packet_r);
    packet_free((Packet*)packet_w);
    free(handshake_packet_body); 

    return c_assert(success, "Could not send handshake packet.\n");
}

bool mc_protocol_get_status(int sock) {
    errno = 0;
    if(!c_assert(sock >= 0, "Passed socket with fd %d", sock)) {
        return false;
    }

    packetReadable* pr = packet_readable_create(
            0x00,
            NULL, 
            0,
            false
        );
    if(!c_assert(pr != NULL, "Could not create readable status packet.")) {
            return false;
    }

    packetWritable* pw = packet_writable_from_readable(pr, false);
    bool success = packet_stream_from_writable(sock, pw);
    packet_free((Packet*)pr);
    packet_free((Packet*)pw);

    return c_assert(success, "Could not write status packet to server.");
}


//packet body format:
//string(16)
//uuid

//NOTE: when sending strings to a minecraft server, don't include
//the null terminator, instead prefix it's length as a varint.
bool mc_protocol_login_request(
        int sock,
        const char* username,
        const UUID_128* uuid
) {
    bool precond_met = c_assert(sock >= 0, "Passed socket with fd %d", sock);
    precond_met     &= c_assert(username != NULL, "Passed NULL username"); 
    if(!precond_met) {
        return false;
    }

    uint64_t uuid_lsb = 0; //lsb = least significant BYTES
    uint64_t uuid_msb = 0;
    if(uuid != NULL) {
        uuid_msb = uuid->msb;
        uuid_lsb = uuid->lsb;
    }

    size_t username_length = strnlen(username, MAX_USERNAME_LENGTH);
    Varint* username_length_vint = varint_int_to_vint(username_length);
    bool error = !c_assert(
            username_length_vint != NULL,
            "Could not convert username length into a varint"
    );
    if(error) {
        return false;
    }

    size_t data_len = sizeof(uint64_t) * 2 + username_length_vint->len + username_length;
    uint8_t buff[data_len];

    size_t offset = 0;

    memcpy(buff + offset, username_length_vint->data, username_length_vint->len);
    offset += username_length_vint->len;
    memcpy(buff + offset, username, username_length);
    offset += username_length;
    memcpy(buff + offset, &uuid_msb, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    memcpy(buff + offset, &uuid_lsb, sizeof(uint64_t));
    offset += sizeof(uint64_t);

    packetReadable* pr = packet_readable_create(LOGIN_REQUEST_PACKET_ID, buff, data_len, false); 
    error = !c_assert(pr != NULL, "Could not create login request packet.");
    if(error) {
        varint_free(username_length_vint);
        return false;
    }
    packetWritable* pw = packet_writable_from_readable(pr, false);
    bool send_success = packet_stream_from_writable(sock, pw);

    varint_free(username_length_vint);
    packet_free((Packet*)pr);
    packet_free((Packet*)pw);

    return c_assert(send_success, "Could not send login packet.");
}

/* encryptionRequestResponseWritable* mc_protocol_parse_encryption_request_response(uint8_t* resp) { */
/*     bool precond_met = c_assert(resp != NULL, "Passed NULL data"); */
/*     if(!precond_met) { */
/*         return NULL; */
/*     } */


/*     return NULL; */
/* } */
