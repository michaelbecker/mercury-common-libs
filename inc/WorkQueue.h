/**
 *  Application interface to the Generic Work Queue Library.
 */
#ifndef WORK_QUEUE_H__
#define WORK_QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  The reference to the Work Queue you create.
 */
typedef void *WORK_QUEUE_HANDLE;

/**
 *  Call to create a Work Queue.
 */
WORK_QUEUE_HANDLE 
CreateWorkQueue(void);

/**
 *  Allow you to change the priority of the worker thread.
 *  ONLY USE THIS IF YOU KNOW WHAT YOU ARE DOING!
 */
WORK_QUEUE_HANDLE
CreateWorkQueueEx(int ThreadPriority);

/**
 *  Destroy a Work queue when you don't need it.
 */
void 
DestroyWorkQueue(WORK_QUEUE_HANDLE WorkQueue);

/**
 *  Callback typedef for your Work Callback.
 */
typedef void(* WorkItemCallback)(void *UserData);

/**
 *  Call to add a work item to the Work Queue.
 */
void 
QueueWorkItem(  WORK_QUEUE_HANDLE WorkQueue,
                WorkItemCallback Callback,
                void *UserData);


#ifdef __cplusplus
    }
#endif


#endif
