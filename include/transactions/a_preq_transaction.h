//
// Created by serbis on 02.08.18.
//

#ifndef MNVP_DRIVER_A_PREQ_TRANSACTION_H
#define MNVP_DRIVER_A_PREQ_TRANSACTION_H


//----------------------------------------------DATA-STATES block-------------------------------------------------------

#include "../../libs/miniakka/includes/fsm.h"

#define st_PreqTransaction_Idle 0
#define st_PreqTransaction_WaitAcks 1

#define dt_PreqTransaction_Uninitialized 0
#define dt_PreqTransaction_WaitingAcks 1


typedef struct dtb_PreqTransaction_Uninitialized {
    uint32_t dest;
    ActorRef *gwPool;
    ActorRef *resultRecipient;
    ActorRef *transactionCtl;
    uint32_t netAdr;
    uint32_t timeout;
    uint16_t minFinds;
    uint16_t ttl;
} dtb_PreqTransaction_Uninitialized;
fsm_data* new_dt_PreqTransaction_Uninitialized(uint32_t dest, ActorRef *gwPool, ActorRef *resultRecipient,
                                               ActorRef *transactionCtl, uint32_t netAdr, uint32_t timeout,
                                               uint16_t minFinds,
                                               uint16_t ttl);

typedef struct dtb_PreqTransaction_WaitingAcks {
    uint32_t tid;
    uint16_t found;
    ActorRef *gwPool;
    ActorRef *resultRecipient;
    ActorRef *transactionCtl;
    uint32_t netAdr;
    uint32_t timeout;
    uint16_t minFinds;
} dtb_PreqTransaction_WaitingAcks;
fsm_data* new_dt_PreqTransaction_WaitingAcks(uint32_t tid, ActorRef *gwPool, ActorRef *resultRecipient,
                                             ActorRef *transactionCtl, uint32_t netAdr, uint32_t timeout,
                                             uint16_t minFinds);

//----------------------------------------------- MESSAGES block--------------------------------------------------------

#define r_PreqTransaction_Found 16544
#define r_PreqTransaction_NotFound 21364


//>>>
ActorMessage* new_r_PreqTransaction_Found(uint16_t stackSize);

//>>>
ActorMessage* new_r_PreqTransaction_NotFound(uint16_t stackSize);

//------------------------------------------- FSM INITIALIZATION block--------------------------------------------------

typedef struct b_PreqTransaction_Initialize {
    uint32_t dest;
    ActorRef *gwPool;
    ActorRef *resultRecipient;
    ActorRef *transactionCtl;
    uint32_t netAdr;
    uint32_t timeout;
    uint16_t minFinds;
    uint16_t ttl;
} b_PreqTransaction_Initialize;
ActorMessage* new_m_PreqTransaction_Initialize(uint32_t dest, ActorRef *gwPool, ActorRef *resultRecipient,
                                         ActorRef *transactionCtl, uint32_t netAdr, uint32_t timeout, uint16_t minFinds,
                                         uint16_t ttl, uint16_t stackSize);


Actor* new_PreqTransaction();

#endif //MNVP_DRIVER_A_PREQ_TRANSACTION_H
