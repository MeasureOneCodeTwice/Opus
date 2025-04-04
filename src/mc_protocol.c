#include "../include/mc_protocol.h"

#include "../include/varint.h"
#include "../include/packet.h"
#include "../include/assertlib.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define MC_PROTOCOL_PROTOCOL_VERSION 769
#define MC_PROTOCOL_MAX_RESPONSE_SIZE 32767

/* enum SERVER_STATE { */
/*     STATE_STATUS   = 1, */
/*     STATE_LOGIN    = 2, */
/*     STATE_TRANSFER = 3 */
/* }; */

//-------------------------------------
//-----PRIVATE UTIL PREDECLARATION-----
//-------------------------------------

uint8_t* get_handshake_packet_body(
        int protocol_version,
        char server_addr[255],
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
uint8_t* get_handshake_packet_body(int protocol_version, char server_addr[255], uint16_t port, int next_state, size_t* buff_len) {

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



bool mc_protocol_handshake_custom(
            int sock,  
            char* server_addr,
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
