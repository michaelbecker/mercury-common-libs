#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include "WorkQueue.h"
#include "LogLib.h"


void WorkQueueCallback1(void *UserData)
{
    WORK_QUEUE_HANDLE W1 = (WORK_QUEUE_HANDLE)UserData;
    static int Count = 0;

    Count++;

    printf("Callback 1: Starting \n");
    LogMessage("Callback 1: Starting ");
    sleep(5);
    printf("Callback 1: re-queing myself\n");
    LogMessage("Callback 1: re-queing myself %d times", Count);

    QueueWorkItem(W1, WorkQueueCallback1, W1);
}


int Cnt2 = 0;
void  WorkQueueCallback2(void *UserData)
{
    printf("Callback 2: Starting\n");
    LogMessage("Callback 2: Starting");
    sleep(1);
    Cnt2++;
    printf("Callback 2: Count = %d\n", Cnt2);
    LogMessage("Callback 2: Count = %d", Cnt2);
}


void  WorkQueueCallback3(void *UserData)
{
    printf("Callback 3: Instant!\n");
    LogMessage("Callback 3: Instant!");
}


void  WorkQueueCallbackDelay(void *UserData)
{
    printf("Callback Delay: Starting\n");
    LogMessage("Callback Delay: Starting");
    
    printf("Thread Priority %d\n", sched_getscheduler(0));

    sleep(1);
    printf("Callback Delay: Ending\n");
    LogMessage("Callback Delay: Ending");
}


int Cnt4 = 0;
void  WorkQueueCallback4(void *UserData)
{
    printf("Callback 4: Starting\n");
    LogMessage("Callback 4: Starting");

    printf("Thread Priority %d\n", sched_getscheduler(0));

    sleep(1);
    Cnt4++;
    printf("Callback 4: Count = %d\n", Cnt2);
    LogMessage("Callback 4: Count = %d", Cnt2);
}




int main(int argc, char *argv[])
{
    InitLog();

    printf("Testing Work Queues...\n");
    LogMessage("Testing Work Queues...");

    sleep(10);
    printf("starting ...\n");

    WORK_QUEUE_HANDLE W1 = CreateWorkQueue();
    WORK_QUEUE_HANDLE W2 = CreateWorkQueue();
    WORK_QUEUE_HANDLE W3 = CreateWorkQueue();

    WORK_QUEUE_HANDLE W4 = CreateWorkQueueEx(55);

    QueueWorkItem(W1, WorkQueueCallback1, W1);

    QueueWorkItem(W2, WorkQueueCallbackDelay, NULL);


    for (int i = 0; i<100; i++){
        QueueWorkItem(W2, WorkQueueCallback2, NULL);
    }


    for (int i = 0; i<10; i++){
        QueueWorkItem(W3, WorkQueueCallback3, NULL);
        QueueWorkItem(W3, WorkQueueCallbackDelay, NULL);
    }


    for (int i = 0; i<20; i++){
        QueueWorkItem(W4, WorkQueueCallback4, NULL);
    }


    printf("Main Thread - All Done...\n");
    LogMessage("Main Thread - All Done...");
    sleep(60);

	return 0;
}

