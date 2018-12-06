//
// Created by serbis on 16.07.18.
//

#ifndef MNVP_DRIVER_A_RTABLE_H
#define MNVP_DRIVER_A_RTABLE_H

#include <stdint.h>
#include "../libs/miniakka/includes/messages.h"
#include "../libs/miniakka/includes/actor.h"

#define m_Rtable_RemoveAllRoutesByGateway 21644
#define m_Rtable_RemoveAllRoutesByDest 16652
#define m_Rtable_UpdateRoute 15495
#define m_Rtable_FindRoute 44654
#define r_Rtable_Route 44654

typedef struct b_Rtable_RemoveAllRoutesByGateway { uint32_t gwAdr; } b_Rtable_RemoveAllRoutesByGateway;
ActorMessage* new_m_Rtable_RemoveAllRoutesByGateway(uint32_t gwAdr, uint16_t stackSize);
typedef struct b_Rtable_UpdateRoute { uint32_t dest; uint32_t gateway; uint16_t gwl; uint16_t distance; } b_Rtable_UpdateRoute;
ActorMessage* new_m_Rtable_UpdateRoute(uint32_t dest, uint32_t gateway, uint16_t gwl, uint16_t distance, uint16_t stackSize);
typedef struct b_Rtable_RemoveAllRoutesByDest { uint32_t dest; } b_Rtable_RemoveAllRoutesByDest;
ActorMessage* new_m_Rtable_RemoveAllRoutesByDest(uint32_t dest, uint16_t stackSize);
typedef struct b_Rtable_FindRoute { uint32_t dest; } b_Rtable_FindRoute;
ActorMessage* new_m_Rtable_FindRoute(uint32_t dest, uint16_t stackSize);
typedef struct b_Rtable_Route { uint32_t gateway; uint16_t gwl; uint16_t dist; } b_Rtable_Route;
ActorMessage* new_r_Rtable_Route(uint32_t gateway, uint16_t gwl, uint16_t dist, uint16_t stackSize);

Actor* new_Rtable();

#endif //MNVP_DRIVER_A_RTABLE_H
