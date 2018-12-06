#include <string.h>
#include "../include/extractors.h"

b_Rtable_FindRoute* b_Rtable_FindRoute_extr(b_Rtable_FindRoute *body) {
    b_Rtable_FindRoute *dup_body = pmalloc(sizeof(b_Rtable_FindRoute));

    dup_body->dest = body->dest;

    return dup_body;
}

b_Gateway_Label* b_Gateway_Label_extr(b_Gateway_Label *body) {
    b_Gateway_Label *dup_body = pmalloc(sizeof(b_Gateway_Label));
    dup_body->label = body->label;

    return dup_body;
}

b_Connection_SendData* b_Connection_SendData_extr(b_Connection_SendData *body) {
    b_Connection_SendData *dup_body = pmalloc(sizeof(b_Connection_SendData));
    uint8_t *dup_data = pmalloc(body->size);
    memcpy(dup_data, body->data, body->size);

    dup_body->size = body->size;
    dup_body->data = dup_data;

    return dup_body;
}

b_SendPacketFsm_SendResult* b_SendPacketFsm_SendResult_extr(b_SendPacketFsm_SendResult *body) {
    b_SendPacketFsm_SendResult *dup_body = pmalloc(sizeof(b_SendPacketFsm_SendResult));

    dup_body->code = body->code;

    return dup_body;
}