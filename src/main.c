//socket imports
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/varint.h"
#include "../include/packet.h"
#include "../include/assertlib.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>



#define SERVER_ADDR "10.0.0.219"
#define SERVER_PORT 25565
#define PROTOCOL_VERSION 769

#define MAX_RESPONSE_SIZE 32767

enum SERVER_STATE {
    STATE_STATUS   = 1,
    STATE_LOGIN    = 2,
    STATE_TRANSFER = 3
};

#define handshake(fd, next_state)\
    handshake_custom( fd, PROTOCOL_VERSION, SERVER_ADDR, SERVER_PORT, next_state )
            

int get_connected_socket(char* address, int port);
uint8_t* get_handshake_packet_body(
        int protocol_version,
        char server_addr[255],
        uint16_t port,
        int next_state,
        size_t* buff_len
);
bool handshake_custom(
            int sock,  
            int protocol_version,
            char* server_addr,
            uint16_t server_port,
            enum SERVER_STATE next_state
);

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

int get_connected_socket(char* address, int port) {

    errno = 0;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(!c_assert(errno == 0, "Could not create socket.")) {
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port  = htons(port);
    int status = inet_aton(address, &addr.sin_addr);
    if(status == 0) {
        close(sock);
        return -1;
    }
    connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    bool success = c_assert(
            errno == 0,
            "Error connecting socket to %s:%d\n",
            address,
            port
        );

    if(!success) {
        close(sock);
        return -1;
    }
    return sock;
}


bool handshake_custom(
            int sock,  
            int protocol_version,
            char* server_addr,
            uint16_t server_port,
            enum SERVER_STATE next_state
) {
    size_t packet_body_len = 0;
    uint8_t* handshake_packet_body = get_handshake_packet_body(
            protocol_version,
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

int main() {

    int sock = get_connected_socket(SERVER_ADDR, SERVER_PORT);
    if(!c_assert(sock >= 0, "Could not get socket.")) {
        return 1;
    }
    
    bool success = handshake(sock, STATE_STATUS);
    if(!c_assert(success, "Handshake failed.")) {
        return 1;        
    }
    
    printf("End of main more coming soon ;).\n");
    /* packetWritable* response_w = packet_writable_from_stream(sock); */
    /* success = c_assert( response_w != NULL, */ 
    /*         "Could not recieve response packet." */
    /* ); */
    /* if(!success) { return 1; } */


    /* packetReadable* response_r = packet_readable_from_writable(response_w, false); */


    /* printf("bytes read: %zu\ncontent:\n", response_r->data_len); */
    /* for(int i = 0; i < response_r->data_len; i++) { */
    /*    printf("%b\n", response_r->data[i]); */ 
    /* } */

}
