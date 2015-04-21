/**
 *  @file   WorkQueue.cpp
 *  @brief  Implementation of a generic WorkQueue library. 
 *
 *  The Library provides a C API to make is as easy as possible to 
 *  interface other code to it. But inside we leverage the STL list 
 *  to make it simple to queue and dequeue items. We handle the 
 *  locking, so this is thread safe.
 */
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "TaWinAssert.h"
#include "WorkQueue.h"
#include "Version.h"

using namespace std;



#define WORK_ITEM_SIGNATURE 'WRKI'

typedef struct WORK_ITEM_ {

    unsigned int Signature;
    WorkItemCallback Callback;
    void *UserData;

}WORK_ITEM, *PWORK_ITEM;



#define WORK_QUEUE_SIGNATURE 'WRKQ'

typedef struct WORK_QUEUE_ {

    unsigned int            Signature;
    list<PWORK_ITEM>        *WorkItems;
    pthread_mutex_t         WorkQueueLock;
    pthread_t               ThreadHandle;
    int                     RequestedThreadPriority;
    sem_t                   ThreadMessageSemaphore;
    bool                    TerminateWorkQueue;

}WORK_QUEUE, *PWORK_QUEUE;



void __attribute__  ((visibility ("default")))
QueueWorkItem(  WORK_QUEUE_HANDLE WorkQueueHandle,
                WorkItemCallback Callback,
                void *UserData)
{
    //---------------------------------
    PWORK_QUEUE WorkQueue;
    PWORK_ITEM WorkItem;
    //---------------------------------

    WorkQueue = (PWORK_QUEUE)WorkQueueHandle;
    ASSERT(WorkQueue->Signature == WORK_QUEUE_SIGNATURE);

    WorkItem = (PWORK_ITEM)malloc(sizeof(WORK_ITEM));
    ASSERT(WorkItem != NULL);

    WorkItem->Signature = WORK_ITEM_SIGNATURE;
    WorkItem->Callback = Callback;
    WorkItem->UserData = UserData;

    pthread_mutex_lock(&WorkQueue->WorkQueueLock);
    WorkQueue->WorkItems->push_back(WorkItem);
    sem_post(&WorkQueue->ThreadMessageSemaphore);
    pthread_mutex_unlock(&WorkQueue->WorkQueueLock);
}



static PWORK_ITEM 
DequeueWorkItem(PWORK_QUEUE WorkQueue)
{
    //-------------------------
    PWORK_ITEM WorkItem;
    //-------------------------

    pthread_mutex_lock(&WorkQueue->WorkQueueLock);

    if (WorkQueue->WorkItems->empty()){
        WorkItem = NULL;
    }
    else{
        WorkItem = WorkQueue->WorkItems->front();
        WorkQueue->WorkItems->pop_front();
    }

    pthread_mutex_unlock(&WorkQueue->WorkQueueLock);

    return WorkItem;
}



static void *
WorkQueueThread( void * lpParam )
{
    //----------------------------------------------
    PWORK_QUEUE WorkQueue;
    PWORK_ITEM WorkItem;
    //----------------------------------------------

    //
    //  We do not need cleanup.
    //
    pthread_detach(pthread_self());

    //
    //  We need to know what our WorkQueue is!
    //
    WorkQueue = (PWORK_QUEUE)lpParam;

    //
    //  Check the signature of the Work Queue
    // 
    pthread_mutex_lock(&WorkQueue->WorkQueueLock);
    
    ASSERT(WorkQueue->Signature == WORK_QUEUE_SIGNATURE);

    pthread_mutex_unlock(&WorkQueue->WorkQueueLock);


    //
    //  If there is a real value in here, attempt to change 
    //  the Priority of the Workqueue.
    //
    if (WorkQueue->RequestedThreadPriority >= 0){

        struct sched_param SchedulerParameters;
        int rc;
        
        memset(&SchedulerParameters, 0, sizeof(SchedulerParameters));

        SchedulerParameters.sched_priority = WorkQueue->RequestedThreadPriority;

        rc = sched_setscheduler (   0,
                                    SCHED_FIFO,
                                    &SchedulerParameters
                                    );
        (void)rc;
    }


    //
    //  Main thread loop
    //
    for (;;){

        sem_wait(&WorkQueue->ThreadMessageSemaphore);

        if (WorkQueue->TerminateWorkQueue){

            do {

                WorkItem = DequeueWorkItem(WorkQueue);
                memset(WorkItem, 0xaa, sizeof(WORK_ITEM));
                free(WorkItem);

            } while (WorkItem != NULL);

            delete WorkQueue->WorkItems;

            pthread_mutex_destroy(&WorkQueue->WorkQueueLock);

            sem_destroy(&WorkQueue->ThreadMessageSemaphore);

            memset(WorkQueue, 0xbb, sizeof(WORK_QUEUE));

            return (0);
        }


        //
        //  Get a Work Item out of the queue
        //                
        WorkItem = DequeueWorkItem(WorkQueue);

        while (WorkItem){

            //
            //  Check the signature
            // 
            ASSERT(WorkItem->Signature == WORK_ITEM_SIGNATURE);

            //
            //  Do the Callback
            // 
            (WorkItem->Callback)(WorkItem->UserData);

            //
            //  We're done, free the memory
            //
            memset(WorkItem, 0xaa, sizeof(WORK_ITEM));
            free(WorkItem);
            
            //
            //  Drain the Work Queue, get another Work Item out of the queue
            //                
            WorkItem = DequeueWorkItem(WorkQueue);
        }
    }

    return (0);
}



WORK_QUEUE_HANDLE __attribute__  ((visibility ("default")))
CreateWorkQueueEx(int ThreadPriority)
{
    //----------------------------------
    PWORK_QUEUE WorkQueue;
    pthread_mutexattr_t attr;
    int rc;
    static bool LogVersion = true;
    //----------------------------------

    WorkQueue = (PWORK_QUEUE)malloc(sizeof(WORK_QUEUE));
    ASSERT(WorkQueue != NULL);

    WorkQueue->Signature = WORK_QUEUE_SIGNATURE;
    WorkQueue->RequestedThreadPriority = ThreadPriority;

    WorkQueue->WorkItems = new list<PWORK_ITEM>;

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&WorkQueue->WorkQueueLock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);

    rc = sem_init(&WorkQueue->ThreadMessageSemaphore, 0, 0);
    ASSERT(rc == 0);

    WorkQueue->TerminateWorkQueue = false;

    rc = pthread_create(&WorkQueue->ThreadHandle, NULL, WorkQueueThread, WorkQueue);
    ASSERT(rc == 0);

    //
    //  Not thread safe if you try realllly hard, but in that case we 
    //  just get 2 messages, not a huge deal.
    //
    if (LogVersion){
        LogVersion = false;
        LOG_LIBRARY_VERSION();
    }

    return (WORK_QUEUE_HANDLE)WorkQueue;
}



WORK_QUEUE_HANDLE __attribute__  ((visibility ("default")))
CreateWorkQueue(void)
{
    //-----------------------------
    WORK_QUEUE_HANDLE hWorkQueue;
    //-----------------------------

    hWorkQueue = CreateWorkQueueEx(-1);

    return hWorkQueue;
}



void __attribute__  ((visibility ("default")))
DestroyWorkQueue(WORK_QUEUE_HANDLE WorkQueueHandle)
{
    //---------------------------------
    PWORK_QUEUE WorkQueue;
    //---------------------------------

    WorkQueue = (PWORK_QUEUE)WorkQueueHandle;
    ASSERT(WorkQueue->Signature == WORK_QUEUE_SIGNATURE);

    WorkQueue->TerminateWorkQueue = true;
    
    sem_post(&WorkQueue->ThreadMessageSemaphore);
}
