#include <string.h>
#include <stdlib.h>
#include <zconf.h>
#include "../includes/testprobe.h"
#include "../../../oscl/include/time.h"
#include "../includes/standard_messages.h"
#include "../includes/actor_utils.h"
#include "../../../oscl/include/data.h"


typedef struct ProbeContext {
    bool pused;
    bool delayExpect;
    uint16_t recMsgType;
    void *recMsgBody;
    void* (*msgExtractor)(void*);
    bool locked; //Флаг блокировки пробки - если true, значит проблка работает в режиме заглушки и не должна принимать сообщения
    ActorRef *lastSender;
    mutex_t *contextMutex;
    mutex_t *topMutex;
} ProbeContext;

void* empty_extr(void *body) {
    return NULL;
}

bool TestProbeActor_receive(Actor *this, uint16_t type, void *msg) {
    ProbeContext *pContext = (ProbeContext*)this->state;

    int r = rand() % 65535;
    //printf("A - %d / msg - %d\n", r, type);

    if (!pContext->locked) {

        switch (type) {
            default:
                MutexLock(pContext->contextMutex);
                if (pContext->pused == false) {
                    pContext->pused = true;

                    while (true) {
                        MutexLock(pContext->topMutex);
                        if (pContext->msgExtractor == NULL) {
                            //printf("null\n");
                            MutexUnlock(pContext->topMutex);
                            usleep(100000); //TODO переделать на кросплатформенную реализацию
                        } else {
                            MutexUnlock(pContext->topMutex);
                            pContext->recMsgBody = pContext->msgExtractor(msg);
                            break;
                        }
                    }

                    /*if (pContext->msgExtractor != NULL) {
                        pContext->recMsgBody = pContext->msgExtractor(msg);
                    } else {
                        pContext->recMsgBody = NULL;
                    }*/

                    if (pContext->lastSender != NULL)
                        AU_freeRef(pContext->lastSender);
                    ActorRef *dup_sender = NULL;
                    if (this->context->sender != NULL) {
                       dup_sender = AU_copyRef(this->context->sender);
                    }
                    pContext->lastSender = dup_sender;
                    pContext->recMsgType = type;

                }

                MutexUnlock(pContext->contextMutex);

                break;
        }
    }

    //printf("B - %d / msg - %d\n", r, type);

    return true;
}


Actor* new_TestProbeActor(bool locked) {
    ProbeContext *context = pmalloc(sizeof(ProbeContext));
    context->pused = false;
    context->delayExpect = false;
    context->recMsgBody = NULL;
    context->recMsgType = 0;
    context->lastSender = NULL;
    context->msgExtractor = NULL;
    context->locked = locked;
    context->contextMutex = NewMutex();
    context->topMutex = NewMutex();

    return new_Actor(TestProbeActor_receive, NULL, context);
}

void* TestProbe_expectMsgType(TestProbe *this, uint16_t type, void* (*msgExtractor)(void*), uint32_t timeout) {
    ProbeContext *pContext = (ProbeContext*)this->actorOject->state;
    MutexLock(pContext->topMutex);
    pContext->msgExtractor = msgExtractor;
    MutexUnlock(pContext->topMutex);

    uint64_t curMs = SystemTime();

    while(pContext->recMsgType == 0 && SystemTime() - curMs < timeout) { usleep(100000); }

    MutexLock(pContext->contextMutex);
    if (pContext->recMsgType == 0) {
        printf("TestProbe [%s] fail by timeout\n", this->name);
        exit(1);
        //return NULL;
    } else {
        if (pContext->recMsgType == type) {
            void* body = pContext->recMsgBody;
            pContext->pused = false;
            pContext->recMsgBody = NULL;
            pContext->recMsgType = 0;
            pContext->msgExtractor = NULL;

            MutexUnlock(pContext->contextMutex);

            return body;
        } else {
            printf("TestProbe [%s] fail by unexpected message type - %d\n", this->name, pContext->recMsgType);
            exit(1);
        }
    }
}

void TestProbe_send(TestProbe *this, ActorRef *target, ActorMessage* message) {
    ((ProbeContext*)this->actorOject->state)->pused = false;
    target->tell(target, message, this->ref, this->targetSystem);
}

void TestProbe_reply(TestProbe *this, ActorMessage* message) {
    ProbeContext *context = (ProbeContext*) this->actorOject->state;
    if (context->lastSender != NULL) {
        context->lastSender->tell(context->lastSender, message, this->ref, this->targetSystem);
    } else {
        printf("TestProbe [%s] last sender is null", this->name);
        exit(1);
    }
}

/*
 * Переводит пробку в режим отоженнного завхвата сообщения. */
void TestProbe_capture(TestProbe *this) {
    ProbeContext *context = (ProbeContext*) this->actorOject->state;

    MutexLock(context->topMutex);
    context->delayExpect = true;
    MutexUnlock(context->topMutex);
}

void TestProbe_destroy(TestProbe *this) {
    ProbeContext *context = (ProbeContext*) this->actorOject->state;

    mutex_t *mutex = context->contextMutex;
    mutex_t *topMutex = context->topMutex;

    MutexLock(this->targetSystem->mutex);
    MAP_remove(this->name, this->targetSystem->actors);
    MutexUnlock(this->targetSystem->mutex);

    MutexLock(mutex);

    if (context->lastSender != NULL)
        AU_freeRef(context->lastSender);
    //if (context->recMsgBody != NULL) //Должен освобождаться кодом теста
        //pfree(context->recMsgBody);
    context->msgExtractor = NULL;

    this->ref->tell(this->ref, new_m_Stop(0), NULL, this->testSystem);

    pfree(this->name);
    AU_freeRef(this->ref);
    MutexUnlock(mutex);
    pfree(mutex);
    pfree(topMutex);

    //pfree(mutex);
}

TestProbe* new_TestProbe(ActorSystem *testSystem, ActorSystem *targetSystem, char* name) {
    TestProbe *testProbe = pmalloc(sizeof(TestProbe));
    testProbe->actorOject = new_TestProbeActor(false);

    uint16_t namePrefix = (uint16_t) (rand() % 65535);
    char *namePrefixStr = itoa(namePrefix);
    size_t namePrefixStrLen = strlen(namePrefixStr);
    size_t nameLen = strlen(name);
    char *finalName = pmalloc(nameLen + 1 + namePrefixStrLen + 1);
    memcpy(finalName, name, nameLen);
    memcpy(finalName + nameLen + 1, namePrefixStr, namePrefixStrLen);
    finalName[nameLen] = '-';
    finalName[nameLen + namePrefixStrLen + 1] = 0;
    pfree(namePrefixStr);

    testProbe->ref = testSystem->actorOf(testSystem, testProbe->actorOject, finalName);
    testProbe->name = finalName;
    testProbe->testSystem = testSystem;
    testProbe->targetSystem = targetSystem;
    testProbe->destroy = (void (*)(void *)) TestProbe_destroy;

    testProbe->expectMsgType = (void *(*)(void *, uint16_t, void *(*)(void *), uint32_t)) TestProbe_expectMsgType;
    testProbe->send = (void (*)(void *, ActorRef *, ActorMessage *)) TestProbe_send;
    testProbe->reply = (void (*)(void *, ActorMessage *)) TestProbe_reply;
    testProbe->capture = (void (*)(void *)) TestProbe_capture;

    MutexLock(targetSystem->mutex);
    MAP_add(finalName, NULL, targetSystem->actors);
    MutexUnlock(targetSystem->mutex);

    return testProbe;
}

TestProbe* new_LockedTestProbe(ActorSystem *testSystem, ActorSystem *targetSystem, char* name) {
    TestProbe *testProbe = pmalloc(sizeof(TestProbe));
    testProbe->actorOject = new_TestProbeActor(true);

    uint16_t namePrefix = (uint16_t) (rand() % 65535);
    char *namePrefixStr = itoa(namePrefix);
    size_t namePrefixStrLen = strlen(namePrefixStr);
    size_t nameLen = strlen(name);
    char *finalName = pmalloc(nameLen + 1 + namePrefixStrLen + 1);
    memcpy(finalName, name, nameLen);
    memcpy(finalName + nameLen + 1, namePrefixStr, namePrefixStrLen);
    finalName[nameLen] = '-';
    finalName[nameLen + namePrefixStrLen + 1] = 0;
    pfree(namePrefixStr);

    testProbe->ref = testSystem->actorOf(testSystem, testProbe->actorOject, finalName);
    testProbe->name = finalName;
    testProbe->testSystem = testSystem;
    testProbe->targetSystem = targetSystem;
    testProbe->destroy = (void (*)(void *)) TestProbe_destroy;

    testProbe->expectMsgType = (void *(*)(void *, uint16_t, void *(*)(void *), uint32_t)) TestProbe_expectMsgType;
    testProbe->send = (void (*)(void *, ActorRef *, ActorMessage *)) TestProbe_send;
    testProbe->reply = (void (*)(void *, ActorMessage *)) TestProbe_reply;
    testProbe->capture = (void (*)(void *)) TestProbe_capture;

    MutexLock(targetSystem->mutex);
    MAP_add(finalName, NULL, targetSystem->actors);
    MutexUnlock(targetSystem->mutex);

    return testProbe;

    /*TestProbe *testProbe = pmalloc(sizeof(TestProbe));
    testProbe->actorOject = new_TestProbeActor(true);

    uint16_t namePrefix = (uint16_t) (rand() % 65535);
    char *namePrefixStr = itoa(namePrefix);
    size_t namePrefixStrLen = strlen(namePrefixStr);
    size_t nameLen = strlen(name);
    char *finalName = pmalloc(nameLen + 1 + namePrefixStrLen + 1);
    memcpy(finalName, name, nameLen);
    memcpy(finalName + nameLen + 1, namePrefixStr, namePrefixStrLen);
    finalName[nameLen] = '-';
    finalName[nameLen + namePrefixStrLen + 1] = 0;
    pfree(namePrefixStr);

    testProbe->ref = testSystem->actorOf(testSystem, testProbe->actorOject, finalName);
    testProbe->name = finalName;
    testProbe->testSystem = testSystem;
    testProbe->targetSystem = targetSystem;
    testProbe->destroy = (void (*)(void *)) TestProbe_destroy;

    testProbe->expectMsgType = (void *(*)(void *, uint16_t, void *(*)(void *), uint32_t)) TestProbe_expectMsgType;
    testProbe->send = (void (*)(void *, ActorRef *, ActorMessage *)) TestProbe_send;
    testProbe->reply = (void (*)(void *, ActorMessage *)) TestProbe_reply;
    testProbe->capture = (void (*)(void *)) TestProbe_capture;

    return testProbe;*/
}