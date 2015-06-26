#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "TaWinAssert.h"
#include "MessageQueue.h"
#include "Timers.h"


enum ThreadMessages{

    MessageInvalid,
    MessageExitThread,
    MessagePrint,
    MessageProcessParallel,
};



void *
PeriodicTestThread(void *Parameter)
{
    //-----------------------------------
    MESSAGE_QUEUE_HANDLE Queue;
    int Id;
    int Length;
    unsigned char Buffer[128];
    int rc;
    struct timespec start;
    struct timespec end;
    struct timespec diff;
    //-----------------------------------

    Queue = (MESSAGE_QUEUE_HANDLE)Parameter;

    for(;;){

        now_timespec(&start);
        Length = 128;
        rc = MqDequeueMessage(Queue, &Id, &Length, Buffer, 0);
        now_timespec(&end);

        if (rc == 0){
            printf("BAD TEST\n");
            exit(-1);
        }
        diff_timespec(&diff, &end, &start);
        printf("Expected = 0  ms-diff = %f  Delta = (%ld, %ld)\n", 
                diff.tv_nsec / (1000.0 *1000.0),  
                diff.tv_sec, diff.tv_nsec);


        now_timespec(&start);
        Length = 128;
        rc = MqDequeueMessage(Queue, &Id, &Length, Buffer, 1);
        now_timespec(&end);

        if (rc == 0){
            printf("BAD TEST\n");
            exit(-1);
        }
        diff_timespec(&diff, &end, &start);
        printf("Expected = 1  ms-diff = %f  Delta = (%ld, %ld)\n",
                diff.tv_nsec / (1000.0 *1000.0),  
                diff.tv_sec, diff.tv_nsec);



        now_timespec(&start);
        Length = 128;
        rc = MqDequeueMessage(Queue, &Id, &Length, Buffer, 3);
        now_timespec(&end);

        if (rc == 0){
            printf("BAD TEST\n");
            exit(-1);
        }
        diff_timespec(&diff, &end, &start);
        printf("Expected = 3  ms-diff = %f  Delta = (%ld, %ld)\n",
                diff.tv_nsec / (1000.0 *1000.0),  
                diff.tv_sec, diff.tv_nsec);

        

        now_timespec(&start);
        Length = 128;
        rc = MqDequeueMessage(Queue, &Id, &Length, Buffer, 5);
        now_timespec(&end);

        if (rc == 0){
            printf("BAD TEST\n");
            exit(-1);
        }
        diff_timespec(&diff, &end, &start);
        printf("Expected = 5  ms-diff = %f  Delta = (%ld, %ld)\n",
                diff.tv_nsec / (1000.0 *1000.0),  
                diff.tv_sec, diff.tv_nsec);

        

        now_timespec(&start);
        Length = 128;
        rc = MqDequeueMessage(Queue, &Id, &Length, Buffer, 7);
        now_timespec(&end);

        if (rc == 0){
            printf("BAD TEST\n");
            exit(-1);
        }
        diff_timespec(&diff, &end, &start);
        printf("Expected = 7  ms-diff = %f  Delta = (%ld, %ld)\n",
                diff.tv_nsec / (1000.0 *1000.0),  
                diff.tv_sec, diff.tv_nsec);

        

        now_timespec(&start);
        Length = 128;
        rc = MqDequeueMessage(Queue, &Id, &Length, Buffer, 10);
        now_timespec(&end);

        if (rc == 0){
            printf("BAD TEST\n");
            exit(-1);
        }
        diff_timespec(&diff, &end, &start);
        printf("Expected = 10  ms-diff = %f  Delta = (%ld, %ld)\n",
                diff.tv_nsec / (1000.0 *1000.0),  
                diff.tv_sec, diff.tv_nsec);



        now_timespec(&start);
        Length = 128;
        rc = MqDequeueMessage(Queue, &Id, &Length, Buffer, 21);
        now_timespec(&end);

        if (rc == 0){
            printf("BAD TEST\n");
            exit(-1);
        }
        diff_timespec(&diff, &end, &start);
        printf("Expected = 21  ms-diff = %f  Delta = (%ld, %ld)\n",
                diff.tv_nsec / (1000.0 *1000.0),  
                diff.tv_sec, diff.tv_nsec);


        return 0;
    }
}



void *
TestThread(void *Parameter)
{
    //-----------------------------------
    MESSAGE_QUEUE_HANDLE Queue;
    int Id;
    int Length;
    unsigned char Buffer[128];
    int rc;
    //-----------------------------------

    Queue = (MESSAGE_QUEUE_HANDLE)Parameter;

    for(;;){

        Length = 128;
        rc = MqDequeueMessage(Queue, &Id, &Length, Buffer, -1);

        if (rc != 0){
            printf("Timeout!\n");
        }
        else{

            switch(Id){

                case MessageProcessParallel:
                    printf("[%08x]Parallel %d\n", (int)pthread_self(), *(int *)Buffer);
                    break;

                case MessagePrint:
                    printf("MessagePrint %s\n", Buffer);
                    break;

                case MessageExitThread:
                    printf("Terminating Thread\n");
                    return 0;
            }
        }
    }
}



int main(int argc, char* argv[])
{
    MESSAGE_QUEUE_HANDLE q1;
    MESSAGE_QUEUE_HANDLE q2;
    MESSAGE_QUEUE_HANDLE q3;
    MESSAGE_QUEUE_HANDLE q4;

    q1 = MqCreateMessageQueue();
    q2 = MqCreateMessageQueue();
    q3 = MqCreateMessageQueue();
    q4 = MqCreateMessageQueue();

    pthread_t threadId;
    int rc;

    rc = pthread_create(&threadId, NULL, TestThread, q1);
    ASSERT(rc == 0);

    rc = pthread_create(&threadId, NULL, TestThread, q2);
    ASSERT(rc == 0);

    rc = pthread_create(&threadId, NULL, TestThread, q3);
    ASSERT(rc == 0);

//#if 0
    for (int i=0; i<10; i++){
        MqQueueMessage(q1, MessagePrint, strlen("Thread1") + 1, (unsigned char *)"Thread1");
        MqQueueMessage(q2, MessagePrint, strlen("Thread2.1") + 1, (unsigned char *)"Thread2.1");
        MqQueueMessage(q2, MessagePrint, strlen("Thread2.2") + 1, (unsigned char *)"Thread2.2");
        MqQueueMessage(q3, MessagePrint, strlen("Thread3.1") + 1, (unsigned char *)"Thread3.1");
        MqQueueMessage(q3, MessagePrint, strlen("Thread3.2") + 1, (unsigned char *)"Thread3.2");
        MqQueueMessage(q3, MessagePrint, strlen("Thread3.3") + 1, (unsigned char *)"Thread3.3");
    }
//#endif
#if 0
    for (int i=0; i<20; i++){
        MqQueueMessage(q1, MessageProcessParallel, sizeof(int), (unsigned char *)&i);
    }
#endif

    sleep(2);

    MqQueueMessage(q1, MessageExitThread, 0, NULL);
    MqQueueMessage(q2, MessageExitThread, 0, NULL);
    MqQueueMessage(q3, MessageExitThread, 0, NULL);

    sleep(2);

    MqDestroyMessageQueue(q1);
    MqDestroyMessageQueue(q2);
    MqDestroyMessageQueue(q3);

    rc = pthread_create(&threadId, NULL, PeriodicTestThread, q4);
    ASSERT(rc == 0);

    sleep(10);

	return 0;
}



