#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct len_data_pair {
    size_t len;
    uint8_t* data;
} lenDataPair;
typedef struct packet_metadata packetMetadata;
typedef struct packet_readable packetReadable;
typedef struct packet_writable packetWritable;


//don't manually write to a packetReadable.
//I have no function to verify if a given 
//packet readable is valid or not, because I
//trust the library to create valid packets.
typedef struct packet_readable {
    packetMetadata* metadata; 
    int             packet_id;   
    uint8_t*        data;
    size_t          data_len;
} packetReadable;

typedef struct packet {
    packetMetadata* metadata; 
} Packet;

//Takes a packetReadable and creates a packetWritable from it.
//Be careful of double frees on the data attribute when 
//using copy_data = false.
packetWritable* packet_writable_from_readable(
        packetReadable* packet, bool copy_data);
//Takes a packetWritable and creates a packetReadable from it.
packetReadable* packet_readable_from_writable(
        packetWritable* packet, bool copy_data);

//Creates a packetReadable. The copy_data flag specifies whether
//to use the given pointer to data in the packetReadable or to copy 
//the contents of the data array
//(which will be freed upon calling packet_free)
packetReadable* packet_readable_create(
        int packet_id,
        uint8_t* data,
        size_t data_len,
        bool copy_data 
); 

bool packet_stream_from_writable(
        int fd, const packetWritable* p
); 
packetWritable* packet_writable_from_stream(int fd);

//destroys a packet object.
//If the packet had copy_data = false, the 
//function will also free the data field of the packet.
void packet_free(Packet *p);

//these functions do not differetiate between packets with 
//copied and non-copied data.
bool packet_readable_eq(
        const packetReadable* p1,
        const packetReadable* p2
);
bool packet_writable_eq(
        const packetWritable* p1,
        const packetWritable* p2
);

//gets the length of the packet writable's body
size_t packet_writable_length(const packetWritable* p);

//takes the data in a packet writable's body and 
//interprets it as a series of varints and data fields
//which it parses it into `num_pairs` lenDataPairs.
//will return null if the contents of the packet body can't be parsed
//to exactly `num_pairs` pairs.
lenDataPair** packet_lenDataPair_array_from_packetReadable_body(
        const packetReadable* packet,
        int num_pairs
);

//frees the lenDataPairs from the above function
void packet_lenDataPair_array_free(
        lenDataPair** pairs,
        int num_pairs
); 

