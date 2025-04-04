#pragma once

#include <stdbool.h>
#include <stdint.h>

enum SERVER_STATE {
    STATE_STATUS   = 1,
    STATE_LOGIN    = 2,
    STATE_TRANSFER = 3
};


#define mc_protocol_handshake(fd, next_state)\
    mc_protocol_handshake_custom( fd, SERVER_ADDR, SERVER_PORT, next_state )

bool mc_protocol_handshake_custom(
            int sock,  
            char* server_addr,
            uint16_t server_port,
            enum SERVER_STATE next_state
);
            
bool mc_protocol_get_status(
            int sock
);


