#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <map>
#include <list>

#include "TaWinAssert.h"
#include "CommonInstrumentInterface.h"
#include "CiiBackend.h"
#include "CommFaultInjectionPrivate.h"
#include "LogLib.h"
#include "WorkQueue.h"
#include "CiiPrivate.h"



/**
 *  Allocate and init bookkeeping for the Message structures we use.
 */
CiiMessage __attribute__  ((visibility ("default"))) * 
CiiAllocateMessage(unsigned int Length)
{
    //-----------------------------
    CiiMessage *Message;
    //-----------------------------

	ASSERT(Length >= 4);

    Message = (CiiMessage *)malloc(sizeof(CiiMessage));
    ASSERT(Message);

    memset(Message, 0, sizeof(CiiMessage));
    Message->Signature = CII_MESSAGE_STRUCT_SIGNATURE;

    Message->MessageData = (CiiMessageData *)malloc(Length);
    ASSERT(Message->MessageData);

    memset(Message->MessageData, 0, Length);
    Message->MessageDataLength = Length;

    return Message;
}



/**
 *  Allocate and init bookkeeping for the Message structures we use.
 *  Need to amke sure that all elements of the structure are 
 *  filled in.
 */
CiiMessage * 
CiiCopyMessage(CiiMessage *Original)
{
    //-----------------------------
    CiiMessage *New;
    //-----------------------------

    New = (CiiMessage *)malloc(sizeof(CiiMessage));
    ASSERT(New);

    New->Signature                  = Original->Signature;
    New->SendDataCallback           = Original->SendDataCallback;
    New->CleanupMessageCallback     = Original->CleanupMessageCallback;
    New->ClientData                 = Original->ClientData;
    New->Flags                      = Original->Flags;
    New->MessageDataLength          = Original->MessageDataLength;
    New->ResponseDataLength         = Original->ResponseDataLength;

    if (New->MessageDataLength){
        ASSERT(Original->MessageData != NULL);
        New->MessageData = (CiiMessageData *)malloc(New->MessageDataLength);
        ASSERT(New->MessageData);
        memcpy(New->MessageData, Original->MessageData, New->MessageDataLength);
    }
    else{
        New->MessageData = NULL;
    }

    if (New->ResponseDataLength){
        ASSERT(Original->ResponseData != NULL);
        New->ResponseData = (CiiMessageData *)malloc(New->ResponseDataLength);
        ASSERT(New->ResponseData);
        memcpy(New->ResponseData, Original->ResponseData, New->ResponseDataLength);
    }
    else{
        New->ResponseData = NULL;
    }

    return New;
}



/**
 *  Free a Message structure + some sanity checking.
 */
void __attribute__  ((visibility ("default")))
CiiFreeMessage(CiiMessage *Message)
{
    ASSERT(Message);
    ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

    if (Message->ResponseData != NULL){
        ASSERT(Message->ResponseDataLength != 0);
        memset(Message->ResponseData, 0xEE, Message->ResponseDataLength);
        free(Message->ResponseData);
    }

    if (Message->MessageData != NULL){
        ASSERT(Message->MessageDataLength != 0);
        memset(Message->MessageData, 0xEE, Message->MessageDataLength);
        free(Message->MessageData);
    }

    if (Message->AckEventInitialized){
        sem_destroy(&Message->AckEvent);
    }

    //
    //  If there is a cleanup function here, call it.
    //
    if (Message->CleanupMessageCallback){
        Message->CleanupMessageCallback(Message->ClientData);
    }

    memset(Message, 0xEE, sizeof(CiiMessage));

    free(Message);
}

