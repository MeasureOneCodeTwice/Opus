//socket imports
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/packet.h"
#include "../include/assertlib.h"
#include "../include/mc_protocol.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>



#define SERVER_ADDR "10.0.0.219"
#define SERVER_PORT 25565
int get_connected_socket(char* address, int port);

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


int main() {

    int sock = get_connected_socket(SERVER_ADDR, SERVER_PORT);
    if(!c_assert(sock >= 0, "Could not get socket.")) {
        return 1;
    }
    
    bool success = mc_protocol_handshake(sock, STATE_STATUS);
    if(!c_assert(success, "Handshake failed.")) {
        return 1;        
    }
    
    success = mc_protocol_get_status(sock);
    if(!c_assert(success, "Could not send status request packet.")) {
        return 1;        
    }

    packetWritable* response_w = packet_writable_from_stream(sock);
    success = c_assert( response_w != NULL, 
            "Could not recieve response packet."
    );
    if(!success) { return 1; }


    packetReadable* response_r = packet_readable_from_writable(response_w, false);


    printf("bytes read: %zu\ncontent:\n", response_r->data_len);
    for(int i = 0; i < (int)response_r->data_len; i++) {
       printf("%c", response_r->data[i]); 
    }

    packet_free((Packet*)response_w);
    packet_free((Packet*)response_r);
}
