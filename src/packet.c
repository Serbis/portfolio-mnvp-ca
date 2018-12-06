#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../include/packet.h"
#include "../oscl/include/malloc.h"

Packet *new_Packet(uint64_t length, uint8_t *body) {
    Packet *packet = pmalloc(sizeof(Packet));
    packet->length = length;
    packet->body = body;
}

void free_Packet(Packet *packet) {
    if (packet->body != NULL)
        pfree(packet->body);
    pfree(packet);
}

Packet *copy_Packet(Packet *packet) {
    Packet *dup_packet = pmalloc(sizeof(Packet));
    uint8_t *dup_body;

    memcpy(dup_packet, packet, sizeof(Packet));
    if (packet != NULL) {
        dup_body = pmalloc(packet->length);
        memcpy(dup_body, packet->body, packet->length);
    } else {
        dup_body = NULL;
    }

    dup_packet->body = dup_body;

    return dup_packet;
}

bool compare_PacketHeader(Packet *packet1, Packet *packet2) {
    if (packet1->version != packet2->version) return false;
    if (packet1->msgId != packet2->msgId) return false;
    if (packet1->type != packet2->type) return false;
    if (packet1->seq != packet2->seq) return false;
    if (packet1->source != packet2->source) return false;
    if (packet1->dest != packet2->dest) return false;
    if (packet1->ttl != packet2->ttl) return false;
    if (packet1->startTtl != packet2->startTtl) return false;
    if (packet1->flags != packet2->flags) return false;
    if (packet1->length != packet2->length) return false;

    return true;
}

Packet *copy_PacketNoBody(Packet *packet) {
    Packet *dup_packet = pmalloc(sizeof(Packet));

    memcpy(dup_packet, packet, sizeof(Packet));

    dup_packet->length = 0;
    dup_packet->body = NULL;

    return dup_packet;
}

uint8_t* Packet_toBinary(Packet *packet, uint16_t *size) {
    uint8_t *bin = (uint8_t*) pmalloc((size_t) (29 + packet->length - 1));

    *bin = 0x1f;

    *(bin + 1) = (packet->version & 0xff000000) >> 24; //VERSION
    *(bin + 2) = (packet->version & 0x00ff0000) >> 16;
    *(bin + 3) = (packet->version & 0x0000ff00) >> 8;
    *(bin + 4) = packet->version & 0x000000ff;

    *(bin + 5) = (packet->msgId & 0xff000000) >> 24; //MSGID
    *(bin + 6) = (packet->msgId & 0x00ff0000) >> 16;
    *(bin + 7) = (packet->msgId & 0x0000ff00) >> 8;
    *(bin + 8) = packet->msgId & 0x000000ff;

    *(bin + 9) = packet->type; //TYPE

    *(bin + 10) = (packet->seq & 0xff00) >> 8; //SEQ
    *(bin + 11) = packet->seq & 0x00ff;

    *(bin + 12) = (packet->source & 0xff000000) >> 24; //SOURCE
    *(bin + 13) = (packet->source & 0x00ff0000) >> 16;
    *(bin + 14) = (packet->source & 0x0000ff00) >> 8;
    *(bin + 15) = packet->source & 0x000000ff;

    *(bin + 16) = (packet->dest & 0xff000000) >> 24; //DEST
    *(bin + 17) = (packet->dest & 0x00ff0000) >> 16;
    *(bin + 18) = (packet->dest & 0x0000ff00) >> 8;
    *(bin + 19) = packet->dest & 0x000000ff;

    *(bin + 20) = (packet->ttl & 0xff00) >> 8; //TTL
    *(bin + 21) = packet->ttl & 0x00ff;

    *(bin + 22) = (packet->startTtl & 0xff00) >> 8; //START_TTL
    *(bin + 23) = packet->startTtl & 0x00ff;

    *(bin + 24) = packet->flags; //FLAGS

    *(bin + 25) = (packet->length & 0xff00) >> 8; //LENGTH
    *(bin + 26) = packet->length & 0x00ff;

    *(bin + 27) = 0x3f; //SM2

    memcpy(bin + 28, packet->body, packet->length);

    *size = 28 + packet->length;

    return bin;
}

void Packet_parsePacketHeader(Packet *packet, uint8_t *buffer, uint16_t size) {
    packet->version = *(buffer + size - 27) << 24 | *(buffer + size - 26) << 16 | *(buffer + size - 25) << 8 | *(buffer + size - 24) << 0; //VERSION
    packet->msgId = *(buffer + size - 23) << 24 | *(buffer + size - 22) << 16 | *(buffer + size - 21) << 8 | *(buffer + size - 20) << 0; //MSGID
    packet->type = *(buffer + size - 19); //TYPE
    packet->seq = *(buffer + size - 18) << 8 | *(buffer + size - 17); //SEQ
    packet->source = *(buffer + size - 16) << 24 | *(buffer + size - 15) << 16 | *(buffer + size - 14) << 8 | *(buffer + size - 13) << 0; //SOURCE
    packet->dest = *(buffer + size - 12) << 24 | *(buffer + size - 11) << 16 | *(buffer + size - 10) << 8 | *(buffer + size - 9) << 0; //DEST
    packet->ttl = *(buffer + size - 8) << 8 | *(buffer + size - 7); //TTL
    packet->startTtl = *(buffer + size - 6) << 8 | *(buffer + size - 5); //START_TTL
    packet->flags = *(buffer + size - 4); //FLAGS
    packet->length = *(buffer + size - 3) << 8 | *(buffer + size - 2); //LENGTH
}

char* Packet_toString(Packet *packet) {
    char *buf = pmalloc(200);

    sprintf(buf, "Packet{version=%d, msgId=%d, type%d, seq=%d, source=%d, dest=%d, ttl=%d, startTtl=%d, flags=%d, length=%d, body=[OPT]}",
            packet->version, packet->msgId, packet->type, packet->seq, packet->source,
            packet->dest, packet->ttl, packet->startTtl, packet->flags, packet->length);

    return buf;
}

char* Packet_toHexString(Packet *packet) {

}

PreqAckBody* Packet_Body_preqAckCreate(uint16_t hops) {
    PreqAckBody *body = pmalloc(sizeof(PreqAckBody));
    body->hops = hops;

    return body;
}

void Packet_Body_preqAckFree(PreqAckBody *body) {
    pfree(body);
}

uint8_t* Packet_Body_preqAckToBinary(PreqAckBody *body, uint16_t *size) {
    uint8_t *bin = pmalloc(2);

    bin[0] = (body->hops & 0xff00) >> 8; //START_TTL
    bin[1] = body->hops & 0x00ff;

    *size = 2;

    return bin;
}

PreqAckBody* Packet_Body_preqAckFromBinary(uint8_t *bin, uint16_t size) {
    PreqAckBody *body = pmalloc(sizeof(PreqAckBody));
    body->hops = bin[0] << 8 | bin[1];

    return body;
}

Packet* Packet_createHelloPacket(uint32_t msgId, uint32_t source) {
    Packet *helloPacket = (Packet*) pmalloc (sizeof(Packet));

    helloPacket->version = PROTOCOL_VERSION;
    helloPacket->msgId = msgId;
    helloPacket->type = PACKET_HELLO;
    helloPacket->seq = 0;
    helloPacket->source = source;
    helloPacket->dest = 0;

    helloPacket->ttl = 1;
    helloPacket->startTtl = 1;
    helloPacket->flags = 0;
    helloPacket->length = 1;
    uint8_t *body = (uint8_t*) pmalloc(1);
    *body = 0;
    helloPacket->body = body;

    return helloPacket;
}

Packet* Packet_createPreqPacket(uint32_t msgId, uint32_t source, uint32_t dest, uint16_t ttl) {
    Packet *helloPacket = (Packet*) pmalloc (sizeof(Packet));

    helloPacket->version = PROTOCOL_VERSION;
    helloPacket->msgId = msgId;
    helloPacket->type = PACKET_PREQ;
    helloPacket->seq = 0;
    helloPacket->source = source;
    helloPacket->dest = dest;

    helloPacket->ttl = ttl;
    helloPacket->startTtl = ttl;
    helloPacket->flags = 0;
    helloPacket->length = 1;
    uint8_t *body = (uint8_t*) pmalloc(1);
    *body = 0;
    helloPacket->body = body;

    return helloPacket;
}

Packet* Packet_ackPreqPacket(Packet *packet, uint16_t hops) {
    PreqAckBody *body = Packet_Body_preqAckCreate(hops);
    uint16_t *size = pmalloc(sizeof(size));
    uint8_t *bin = Packet_Body_preqAckToBinary(body, size);

    Packet *ackPacket = copy_PacketNoBody(packet);
    ackPacket->source = packet->dest;
    ackPacket->dest = packet->source;
    ackPacket->ttl = packet->startTtl;
    ackPacket->flags = 0x01;
    ackPacket->body = bin;
    ackPacket->length = *size;

    Packet_Body_preqAckFree(body);
    pfree(size);

    return ackPacket;
}