#include <sys/socket.h>
#include <string.h>
#include "../include/rec_thread.h"
#include "../../libs/miniakka/includes/standard_messages.h"

// ||| Closed (PLAIN) ||| ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ActorMessage* new_r_RecThread_Closed() {
    return new_ActorMessage(r_RecThread_Closed, NULL, 64, del_default);
}

// ||| Closed (BODY) ||| ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void del_r_RecThread_Data(b_RecThread_Data *msg) {
    pfree(msg->data);
}

ActorMessage* new_r_RecThread_Data(uint8_t *data, uint16_t size, uint16_t stackSize) {
    b_RecThread_Data *body = pmalloc(sizeof(b_RecThread_Data));
    body->data = pmalloc(size);
    memcpy(body->data, data, size);
    body->size = size;

    return new_ActorMessage(r_RecThread_Data, body, stackSize, del_r_RecThread_Data);
}



void RecThread_run(void *args) {
    RecThreadArgs *tArgs = args;

    uint8_t *buf = pmalloc(tArgs->blockSize);

    while(!tArgs->stop) {
        uint16_t numbytes;

        if (tArgs->creator != NULL) {
            if ((numbytes = (uint16_t) recv(tArgs->socket, buf, tArgs->blockSize, 0)) == -1) {
                tArgs->creator->tell(tArgs->creator, new_r_RecThread_Closed(), NULL, tArgs->system);
                printf("xx");
                break;
            }

            if (numbytes > 0) {
                uint8_t *nBuf = pmalloc(numbytes);
                memcpy(nBuf, buf, numbytes);
                tArgs->creator->tell(tArgs->creator, new_r_RecThread_Data(nBuf, numbytes, 64), NULL, tArgs->system);
            } else {
                if (numbytes < 0) {
                    printf("xxx");
                }
            }
        }
    }
    pfree(buf);
}