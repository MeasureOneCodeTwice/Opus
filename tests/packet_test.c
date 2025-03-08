#include "../include/testlib.h"
#include "../include/packet.h"
#include "../include/assertlib.h"

#include <stdio.h>
#include <assert.h>
#include <unistd.h>


int tests_passed = 0;
int tests_failed = 0;

#define min(a,b) a < b ? a : b
const int MAX_EXPECTED_BINARY_LENGTH = 5012;

void packet_test(
        uint8_t packet_id,     
        uint8_t* packet_data,
        size_t packet_data_size,
        uint8_t* expected_binary, 
        int expected_binary_length
);

int main(void) {

    uint8_t *expected_binary; 
    uint8_t *buff;

    printf("---- Test set 1 ----\n");
    expected_binary = (uint8_t[2]){ 0x01, 0x00 };
    packet_test(0x00, NULL, 0, expected_binary, 2);

    printf("---- Test set 2 ----\n");
    buff = (uint8_t[1]){ 0x69 };
    expected_binary = (uint8_t[3]){ 0x02, 0x01, 0x69 };
    packet_test(0x01, buff, 1, expected_binary, 3);

    printf("---- Test set 4 ----\n");
    buff = NULL;
    expected_binary = (uint8_t[3]){ 0x02, 0x80, 0x01 };
    packet_test(0x80, buff, 0, expected_binary, 3);

    printf("---- Test set 5 ----\n");
    buff = (uint8_t[5]){ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    expected_binary = (uint8_t[8]){ 0x07, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    packet_test(0x80, buff, 5, expected_binary, 8);

    return summary(&tests_passed, &tests_failed);

}

// -----------------------------------------------------------------------------------------------------
// |  PACKET READABLE 1 --> PACKET WRITABLE 1 --> STREAM --> PACKET WRITABLE 2 --> PACKET READABLE 2   |
// -----------------------------------------------------------------------------------------------------
//This function tests if PACKET READABLE 1 and PACKET READABLE 2 are equal as well as if
//PACKET WRITABLE 1 and PACKET WRITABLE 2 are equal.
//Optionally, the user can provide a uint8_t buffer to confirm if the binary stream is correct 
void packet_test(
        uint8_t packet_id,     
        uint8_t* packet_data,
        size_t packet_data_size,
        uint8_t* expected_binary, 
        int expected_binary_length
){

    bool precond_met = c_assert(
            expected_binary != NULL || expected_binary_length > 0,
            "expected_binary connot be NULL with expected_binary_length > 0 (%d)\n",
            expected_binary_length 
    );
    if(!precond_met) {
        return; 
    }

    int pipes[2];
    bool success = pipe(pipes);
    if(!c_assert(
            !success,
            "Testing error: Could not create pipe")
    ) {
        return;
    }
    int outpipe = pipes[0];
    int inpipe  = pipes[1]; 


    packetReadable* packet_readable_1 = packet_readable_create(
        packet_id,
        packet_data,
        packet_data_size,
        false 
    );
    
    packetWritable* packet_writable_1 = packet_writable_from_readable(
        packet_readable_1,
        false
    );

    packet_stream_from_writable(inpipe, packet_writable_1);


    if(expected_binary_length != -1) {
        size_t total_packet_binary_length = packet_writable_length(packet_writable_1);

        uint8_t packet_binary[total_packet_binary_length];

        errno = 0;
        int bytes_read = read(outpipe, packet_binary, total_packet_binary_length);
        bool failure = !c_assert(errno == 0, "Error reading packet binary from stream");
        failure |= !c_assert(
                bytes_read == (int)total_packet_binary_length, 
                "Packet bytes read and binary length mismatch (%d/%zu)",
                bytes_read,
                total_packet_binary_length
        );

        if(!failure) {
            int bytes_written = write(inpipe, packet_binary, total_packet_binary_length);
            failure = !c_assert(errno == 0, "Error writing packet binary to stream");
            failure |= !c_assert(
                    bytes_written == (int)total_packet_binary_length, 
                    "Packet bytes written and binary length mismatch (%d/%zu)",
                    bytes_read,
                    total_packet_binary_length
            );
        }

        if(failure) { 
            close(inpipe);
            close(outpipe);
            packet_free((Packet*)packet_readable_1);
            packet_free((Packet*)packet_writable_1);
            return;
        };


        bool binary_lengths_equal = expected_binary_length == (int)total_packet_binary_length; 
        verify( binary_lengths_equal, 
                "Length of packet binary does not match expected. (%zu/%d)",
                total_packet_binary_length,
                expected_binary_length 
        );


        bool binary_equal = binary_lengths_equal && 
            memcmp(
                    packet_binary,
                    expected_binary,
                    min(expected_binary_length, (int)total_packet_binary_length)
               ) == 0;


        verify(binary_equal, "Contents of packet binary do not match expected contents.\n");


        if(!binary_equal && binary_lengths_equal) {
            printf("Packet binary:\n");
            for(int i = 0; !binary_equal && i < expected_binary_length; i++) {
                printf("%x ", packet_binary[i]);
                if(i % 4 == 0 && i != 0) {
                    printf("\n");
                }
            }
        }
    }


    packetWritable* packet_writable_2 = packet_writable_from_stream(outpipe);
    verify(packet_writable_eq(packet_writable_1, packet_writable_2), 
            "Reconstructed packetWritable does not match original packetWritable."
    );


    packetReadable* packet_readable_2 = packet_readable_from_writable(
        packet_writable_2, false
    );
    verify(packet_writable_eq(packet_writable_1, packet_writable_2),
            "Reconstructed packetWritable does not match original packetWritable."
    );

    close(inpipe);
    close(outpipe);
    packet_free((Packet*)packet_readable_1);
    packet_free((Packet*)packet_writable_1);
    packet_free((Packet*)packet_readable_2);
    packet_free((Packet*)packet_writable_2);
}
