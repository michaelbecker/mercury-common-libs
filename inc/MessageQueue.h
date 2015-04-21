#ifndef MESSAGE_QUEUE_H__
#define MESSAGE_QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef void * MESSAGE_QUEUE_HANDLE; 

/**
 *  Create a message queue.
 */
MESSAGE_QUEUE_HANDLE 
MqCreateMessageQueue(void);

/**
 *  Delete a message queue.
 */ 
void 
MqDestroyMessageQueue(MESSAGE_QUEUE_HANDLE MessageQueueHandle);

/**
 *  Send a message into the Queue.
 */ 
void 
MqQueueMessage( MESSAGE_QUEUE_HANDLE MessageQueueHandle,
                int Id,
                int Length,
                unsigned char *Data);

/**
 *  Pull a message out of the queue.
 */
int 
MqDequeueMessage(   MESSAGE_QUEUE_HANDLE MessageQueueHandle,
                    int *Id,                        /**< [OUT] Returns Id */
                    int *BufferLength,              /**< [IN / OUT]  Pass in size of Data, passes out the amount of data copied into Data.*/ 
                    unsigned char *Data,            /**< [OUT] Output buffer */
                    int TimeoutInMs);               /**< How long to wait, can be INFINITE */


#ifdef __cplusplus
    }
#endif

#endif 

