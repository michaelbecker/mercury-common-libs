#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#include <list>

#include "TaWinAssert.h"
#include "MessageQueue.h"
#include "Timers.h"
#include "Version.h"


using namespace std;


#define MESSAGE_SIGNATURE 0x2047534D
#define MESSAGE_QUEUE_SIGNATURE 0x5147534D

/**
 *  Internal structure for queueing messages.
 */
typedef struct MQ_MESSAGE_ {

    unsigned int Signature;
    int Id;                 /**< Message ID */
    int Length;             /**< Optional length, needed if there is data */
    unsigned char Data[1];  /**< Optional Data */

}MQ_MESSAGE, *PMQ_MESSAGE;

/**
 *  Opaque structure that handles message queues.
 */
typedef struct MESSAGE_QUEUE_ {

    unsigned int Signature;
    sem_t WaitSemaphore;     
    pthread_mutex_t Lock;
    list<PMQ_MESSAGE> *Messages;

}MESSAGE_QUEUE, *PMESSAGE_QUEUE;



/**
 *  Create a message queue.
 */
MESSAGE_QUEUE_HANDLE __attribute__  ((visibility ("default")))
MqCreateMessageQueue(void)
{
    //---------------------------
    PMESSAGE_QUEUE  Queue;
    int rc;
    pthread_mutexattr_t attr;
    //---------------------------

    Queue = (PMESSAGE_QUEUE)malloc(sizeof(MESSAGE_QUEUE));
    if (!Queue){
        return NULL;
    }

    Queue->Signature = MESSAGE_QUEUE_SIGNATURE;

    rc = sem_init(&Queue->WaitSemaphore, 0, 0);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&Queue->Lock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);

    Queue->Messages = new list<PMQ_MESSAGE>();

    LogVersion();

    return (MESSAGE_QUEUE_HANDLE)Queue;
}



void __attribute__  ((visibility ("default")))
MqDestroyMessageQueue(MESSAGE_QUEUE_HANDLE MessageQueueHandle)
{
    //------------------------------
    PMESSAGE_QUEUE  Queue;
    PMQ_MESSAGE Message;
    //------------------------------

    ASSERT(MessageQueueHandle != NULL);

    Queue = (PMESSAGE_QUEUE)MessageQueueHandle;
    ASSERT(Queue->Signature == MESSAGE_QUEUE_SIGNATURE);

    //
    //  LOCK ---------------------------------------------
    //
    pthread_mutex_lock(&Queue->Lock);

    while(!Queue->Messages->empty()){
        Message = Queue->Messages->front();
        Queue->Messages->pop_front();
        free(Message);
    }

    //
    //  UNLOCK -------------------------------------------
    //
    pthread_mutex_unlock(&Queue->Lock);

    pthread_mutex_destroy(&Queue->Lock);
    delete Queue->Messages;

    memset(Queue, 0xCC, sizeof(MESSAGE_QUEUE));

    free(Queue);
}



void __attribute__  ((visibility ("default")))
MqQueueMessage( MESSAGE_QUEUE_HANDLE MessageQueueHandle,
                int Id,
                int Length,
                unsigned char *Data)
{
    //------------------------------
    PMQ_MESSAGE Message;
    PMESSAGE_QUEUE  Queue;
    //------------------------------

    Queue = (PMESSAGE_QUEUE)MessageQueueHandle;

    ASSERT(Length >= 0);
    if (Length > 0){
        ASSERT(Data != NULL);
    }

    Message = (PMQ_MESSAGE)malloc(sizeof(MQ_MESSAGE) + Length - 1);
    ASSERT(Message != NULL);

    Message->Signature = MESSAGE_SIGNATURE;
    Message->Id = Id;
    Message->Length = Length;

    if (Length){
        memcpy(Message->Data, Data, Length);
    }

    //
    //  LOCK ---------------------------------------------
    //
    pthread_mutex_lock(&Queue->Lock);

    Queue->Messages->push_back(Message);

    //
    //  UNLOCK -------------------------------------------
    //
    pthread_mutex_unlock(&Queue->Lock);

    sem_post(&Queue->WaitSemaphore);
}



int __attribute__  ((visibility ("default")))
MqDequeueMessage(   MESSAGE_QUEUE_HANDLE MessageQueueHandle,
                    int *Id,                        /**< [OUT] Returns Id */
                    int *BufferLength,              /**< [IN / OUT]  Pass in size of Data, passes out the amount of data copied into Data.*/ 
                    unsigned char *Data,            /**< [OUT] Output buffer */
                    int TimeoutInMs)                /**< How long to wait, can be -1 (INFINITE) */
{
    //------------------------------
    PMQ_MESSAGE Message;
    PMESSAGE_QUEUE Queue;
    int MaxLength;
    int rc;
    struct timespec Delay;
    //------------------------------

    ASSERT(MessageQueueHandle != NULL);

    Queue = (PMESSAGE_QUEUE)MessageQueueHandle;
    ASSERT(Queue->Signature == MESSAGE_QUEUE_SIGNATURE);

CHECK_AGAIN:

    if (TimeoutInMs > 0){

        rc = sem_trywait(&Queue->WaitSemaphore);
        
        if ((rc != 0) && (errno == EAGAIN)){
    
            if (TimeoutInMs >= 20){
                set_timespec_from_ms(&Delay, 20);
                TimeoutInMs -= 20;
            }
            else if (TimeoutInMs >= 10){
                set_timespec_from_ms(&Delay, 10);
                TimeoutInMs -= 10;
            }
            else if (TimeoutInMs >= 5){
                set_timespec_from_ms(&Delay, 5);
                TimeoutInMs -= 5;
            }
            else {
                set_timespec_from_ms(&Delay, TimeoutInMs);
                TimeoutInMs = 0;
            }

            nanosleep(&Delay, NULL);
            goto CHECK_AGAIN;
        }

    }
    else if (TimeoutInMs == 0){
        rc = sem_trywait(&Queue->WaitSemaphore);
    }
    else{
        rc = sem_wait(&Queue->WaitSemaphore);
    }


    if (rc == 0){

        //
        //  LOCK ---------------------------------------------
        //
        pthread_mutex_lock(&Queue->Lock);

        Message = Queue->Messages->front();
        Queue->Messages->pop_front();

        //
        //  UNLOCK -------------------------------------------
        //
        pthread_mutex_unlock(&Queue->Lock);

        *Id = Message->Id;
        
        if (*BufferLength < 0){
            *BufferLength = 0;
        }

        if (Message->Length > *BufferLength){
            MaxLength = *BufferLength;
        }
        else {
            MaxLength = Message->Length;
            *BufferLength = MaxLength;
        }

        if (MaxLength > 0){
            memcpy(Data, Message->Data, MaxLength);
        }

        free(Message);
    }

    return rc;
}


