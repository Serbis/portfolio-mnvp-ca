//
// Created by serbis on 13.07.18.
//

#ifndef MNVP_DRIVER_PACKET_H
#define MNVP_DRIVER_PACKET_H


#include <stdint.h>

#define PROTOCOL_VERSION 1

#define PACKET_HELLO 0
#define PACKET_NETWORK_ERROR 1
#define PACKET_PREQ 2
#define PACKET_BINARY 3
#define PACKET_ECHO 4
#define PACKET_RPC 5

typedef struct Packet {
    uint32_t version;
    uint32_t msgId;
    uint8_t type;
    uint16_t seq;
    uint32_t source;
    uint32_t dest;
    int16_t ttl;
    int16_t startTtl;
    uint8_t flags;
    uint16_t length;
    uint8_t *body;
} Packet;

typedef struct PreqAckBody { uint16_t hops; } PreqAckBody;

Packet *new_Packet(uint64_t length, uint8_t *body);
void free_Packet(Packet *packet);
Packet *copy_Packet(Packet *packet);
Packet *copy_PacketNoBody(Packet *packet);
bool compare_PacketHeader(Packet *packet1, Packet *packet2);
uint8_t* Packet_toBinary(Packet *packet, uint16_t *size);
void Packet_parsePacketHeader(Packet *packet, uint8_t *buffer, uint16_t size);
char* Packet_toString(Packet *packet);

PreqAckBody* Packet_Body_preqAckCreate(uint16_t hops);
void Packet_Body_preqAckFree(PreqAckBody *body);
uint8_t* Packet_Body_preqAckToBinary(PreqAckBody *body, uint16_t *size);
PreqAckBody* Packet_Body_preqAckFromBinary(uint8_t *bin, uint16_t size);

Packet* Packet_createHelloPacket(uint32_t msgId, uint32_t source);
Packet* Packet_createPreqPacket(uint32_t msgId, uint32_t source, uint32_t dest, uint16_t ttl);
Packet* Packet_ackPreqPacket(Packet *packet, uint16_t hops);

#endif //MNVP_DRIVER_PACKET_H
