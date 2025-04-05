#pragma once

#include <stdbool.h>
#include <stdint.h>

enum SERVER_STATE {
    STATE_STATUS   = 1,
    STATE_LOGIN    = 2,
    STATE_TRANSFER = 3
};

typedef struct uuid {
    uint64_t msb;
    uint64_t lsb;
} UUID_128;


#define mc_protocol_handshake(fd, next_state)\
    mc_protocol_handshake_custom( fd, SERVER_ADDR, SERVER_PORT, next_state )

bool mc_protocol_handshake_custom(
            int sock,  
            const char* server_addr,
            uint16_t server_port,
            enum SERVER_STATE next_state
);
            
bool mc_protocol_get_status(
            int sock
);

//uuid is unused in notchian servers, can just pass NULL.
bool mc_protocol_login_request(
        int sock,
        const char* username, //any character after the 16th is ignored.
        const UUID_128* uuid  
);
