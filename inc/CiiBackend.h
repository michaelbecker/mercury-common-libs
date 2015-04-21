/**
 *  Backend interface to the Common Instrument Interface Library.
 *  This is for various different low comm protocols and clients, etc.
 */
#ifndef CII_BACKEND_H__
#define CII_BACKEND_H__


#include <semaphore.h>
#include "WorkQueue.h"
#include "CommonInstrumentInterface.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 *  Generic Function to send data.
 */
typedef void (*SendDataFcn_t)(void *ClientData, CiiMessageData *Data, unsigned int Length);


/**
 *  Generic function to clean up Client specific message data.
 */
typedef void (*CleanupMessageFcn_t)(void *ClientData);


/**
 *  Generic Function to terminate a backend connection.
 */
typedef void (*CloseBackendFcn_t)(void *ClientData);


typedef void (*IncrementReferenceFcn_t)(void *ClientData);
typedef void (*DecrementReferenceFcn_t)(void *ClientData);



/**
 *  A Common Instrument Interface Message structure.
 */
typedef struct CiiMessage_ {

    unsigned int        Signature;
    SendDataFcn_t       SendDataCallback;
    CleanupMessageFcn_t CleanupMessageCallback;

    /**
     *  Client specific data - it needs to figure it out.
     */
    void            *ClientData;
    sem_t           AckEvent;
    int             AckEventInitialized;
    unsigned int    Flags;

    CiiMessageData  *MessageData;
    unsigned int    MessageDataLength;

    CiiMessageData *ResponseData;
    unsigned int    ResponseDataLength;

}CiiMessage;


/**
 *  This is an opaque handle.
 */
typedef void * CII_BACK_END_HANDLE;


/**
 *  A backend client calls this when it receives a new message from 
 *  an external client, i.e. TRIOS.
 */
void 
CiiBackendRouteReceivedMessage(CII_BACK_END_HANDLE ClientHandle, CiiMessage *Message);


/**
 *  A backend calls this to register with the Cii.
 */
CII_BACK_END_HANDLE 
CiiRegisterBackend( void *ClientData,               //  Generic Client Data Ptr
                    SendDataFcn_t SendDataCallback, //  Send function
                    CloseBackendFcn_t CloseBackend, //  Close Backend function
                    IncrementReferenceFcn_t IncBackendRef,
                    DecrementReferenceFcn_t DecBackendRef
                    );


/**
 *  If a backend is going away, it has to call this to unregister 
 *  itself.
 */ 
void 
CiiUnregisterBackend(CII_BACK_END_HANDLE ClientHandle);


/**
 *  Allocate a message to get the ball rolling.
 *  You MUST check for NULL.
 */
CiiMessage * 
CiiAllocateMessage(unsigned int Length);


/**
 *  Free a message.  Typically only to be done by the CII itself.
 */
void 
CiiFreeMessage(CiiMessage *Message);



#ifdef __cplusplus
    }
#endif


#endif
