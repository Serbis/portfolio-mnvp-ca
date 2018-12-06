//
// Created by serbis on 09.08.18.
//

#ifndef MNVP_DRIVER_EXTRACTORS_H
#define MNVP_DRIVER_EXTRACTORS_H

#include "../../../include/a_rtable.h"
#include "../../../include/a_gateway.h"
#include "../../../oscl/include/connection.h"
#include "../../../include/a_send_packet_fsm.h"

b_Rtable_FindRoute* b_Rtable_FindRoute_extr(b_Rtable_FindRoute *body);
b_Gateway_Label* b_Gateway_Label_extr(b_Gateway_Label *body);
b_Connection_SendData* b_Connection_SendData_extr(b_Connection_SendData *body);
b_SendPacketFsm_SendResult* b_SendPacketFsm_SendResult_extr(b_SendPacketFsm_SendResult *body);

#endif //MNVP_DRIVER_EXTRACTORS_H
