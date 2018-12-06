//
// Created by serbis on 14.07.18.
//

#ifndef MNVP_DRIVER_ACTOR_POOL_H
#define MNVP_DRIVER_ACTOR_POOL_H

#include <stdint.h>
#include "../../collections/includes/list.h"
#include "messages.h"
#include "actor_ref.h"

#define ROUND_ROBIN_LOGIC 0

typedef struct RoundRobinRoutingLogic {
    uint8_t nextRoutee;
} RoundRobinRoutingLogic;

typedef struct Router {
    uint8_t routingLogicType;
    mutex_t *mutex;
    void *routingLogic;
    List *routees; //Вообще не очень правильно тут использовать List, тут долже быть Vector
    void (*route)(void*, ActorMessage*, ActorRef*, void*);

} Router;

RoundRobinRoutingLogic* new_RoundRobinRoutingLogic();
Router* new_Router(uint8_t routingLogicType, void *routingLogic, List *routees);

#endif //MNVP_DRIVER_ACTOR_POOL_H
