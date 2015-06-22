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
 *  List of all the Clients that are registered to us.
 */
static list<CiiClientRegistration_t *>ClientList;

/**
 *  Lock for ClientList and IsMasterSet
 */
pthread_mutex_t ClientListLock;

/**
 *	Work queue to decouple the sending of the message from the caller.
 */
WORK_QUEUE_HANDLE StatusWorkQueue = NULL;



void *
CiiStatusThread( void * lpParam )
{
    //----------------------------------------------
    CiiClientRegistration_t *Client;
    CiiMessage *Message;
    int rc;
    //----------------------------------------------

    Client = (CiiClientRegistration_t *)lpParam;
    ASSERT(Client->Signature == CII_BACKEND_SIGNATURE);

    //
    //  We do not need cleanup.
    //
    pthread_detach(pthread_self());

    LogMessage("CiiStat: New Status thread - TID: %p", 
                (void *)Client->Status.ThreadId);

    //
    //  Prevent the backend from going away, take out an extra 
    //  reference to it since we are using it here.
    //
    Client->IncBackendRef(Client->ClientData);

    for(;;){

        //
        //  Block forever waiting for something interesting to do.
        //
        rc = sem_wait(&Client->Status.ThreadMessageSemaphore);
        ASSERT(rc == 0);

        //
        //  LOCK ------------------------------------------
        //
        pthread_mutex_lock(&Client->Status.Lock);
        
        if (Client->Status.TerminateThread){

TERMINATE_IN_PROGRESS:            
            //
            //  UNLOCK ------------------------------------
            //
            pthread_mutex_unlock(&Client->Status.Lock);

            //
            //  All done, drop the referece to the backend.
            //
            Client->DecBackendRef(Client->ClientData);

            LogDisconnect(Client);

            while (!Client->Status.Messages->empty()){

                Message = Client->Status.Messages->back();
                Client->Status.Messages->pop_back();

                ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

                CiiFreeMessage(Message);
            }
            
            delete Client->Status.Messages;

            pthread_mutex_destroy(&Client->Status.Lock);

            sem_destroy(&Client->Status.ThreadMessageSemaphore);

            LogMessage("CiiStat: Terminating Status thread - TID: %p", 
                        (void *)Client->Status.ThreadId);

            memset(Client, 0xEE, sizeof(CiiClientRegistration_t));

            free(Client);

            return (0);
        }
        
        while (!Client->Status.Messages->empty()){

            Message = Client->Status.Messages->back();
            Client->Status.Messages->pop_back();

            ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

            //
            //  UNLOCK ------------------------------------
            //
            pthread_mutex_unlock(&Client->Status.Lock);

		    Client->SendDataCallback(   Client->ClientData, 
                                        Message->MessageData, 
                                        Message->MessageDataLength);
            CiiFreeMessage(Message);

            //
            //  LOCK --------------------------------------
            //
            pthread_mutex_lock(&Client->Status.Lock);

            if (Client->Status.TerminateThread){
                goto TERMINATE_IN_PROGRESS;
            }
        }

        //
        //  UNLOCK ----------------------------------------
        //
        pthread_mutex_unlock(&Client->Status.Lock);

    }
}



#define MAX_STATUS_QUEUE_SIZE   1000



/**
 *  Work queue item here - decouple scheduling of status to a worker thread. 
 */
static void 
RouteStatusMessageToClientThreads(void * UserData)
{
    //----------------------------------------------
    list<CiiClientRegistration_t *>::iterator it;
	CiiClientRegistration_t *Client;
	CiiMessage *Message;
    CiiMessage *Copy;
	//----------------------------------------------

	Message = (CiiMessage *)UserData;
	ASSERT(Message);
	ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

    //
    //  LOCK ---------------------------------------
    //
    pthread_mutex_lock(&ClientListLock);

    for (it = ClientList.begin(); it != ClientList.end(); it++){

        Client = *it;

        ASSERT(Client->Signature == CII_BACKEND_SIGNATURE);

        Copy = CiiCopyMessage(Message);

        ASSERT(Copy->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

        //
        //  LOCK ---------------------------------------
        //
        pthread_mutex_lock(&Client->Status.Lock);

        if (Client->Status.Messages->size() >= MAX_STATUS_QUEUE_SIZE){
            //
            //  Too many, tear down!
            //
            LogMessage("CiiStat: Client queue too deep! %d Messages", 
                        Client->Status.Messages->size());

            Client->CloseBackend(Client->ClientData);
        }
        else if (!Client->Status.TerminateThread){

            Client->Status.Messages->push_front(Copy);
            sem_post(&Client->Status.ThreadMessageSemaphore);
        }

        //
        //  UNLOCK ---------------------------------------
        //
        pthread_mutex_unlock(&Client->Status.Lock);
    }

    //
    //  UNLOCK ---------------------------------------
    //
    pthread_mutex_unlock(&ClientListLock);

    CiiFreeMessage(Message);
}



/**
 *  Universal call to send a status message out to clients.  This call will 
 *  multiplex the message out to all registered back-ends.
 */
void __attribute__  ((visibility ("default")))
CiiSendStatus(unsigned int SubStatus, unsigned char *Data, unsigned int DataLength)
{
    //----------------------------------------------
    CiiMessage *Message;
    //----------------------------------------------

    ASSERT( IS_STATUS_MSG(SubStatus) );

    Message = CiiAllocateMessage(STATUS_HEADER_SIZE + DataLength);
    ASSERT(Message != NULL);

    Message->MessageData->Type = MtStatus;
    Message->MessageData->Status.SubStatus = SubStatus;

    if ((DataLength > 0) && Data){
        memcpy(Message->MessageData->Status.Data, Data, DataLength);
    }

	//
	//	Do it later...
	//
	QueueWorkItem(	StatusWorkQueue,
					RouteStatusMessageToClientThreads,
					Message);
}



/**
 *
 */
void
CreateStatusWorker(CiiClientRegistration_t *Client)
{
    //----------------------
    pthread_mutexattr_t attr;
    int rc;
    //----------------------

    Client->Status.Messages = new list<CiiMessage *>;

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&Client->Status.Lock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);


    rc = sem_init(&Client->Status.ThreadMessageSemaphore, 0, 0);
    ASSERT(rc == 0);

    rc = pthread_create(&Client->Status.ThreadId, NULL, CiiStatusThread, Client);
    ASSERT(rc == 0);

    //
    //  LOCK ----------------------------------------------------------------
    //
    pthread_mutex_lock(&ClientListLock);

    ClientList.push_front(Client);

    LogMessage("CiiStat: New Client Added.  Current Total: %d", ClientList.size());

    //
    //  UNLOCK --------------------------------------------------------------
    //
    pthread_mutex_unlock(&ClientListLock);
}



/**
 *
 */
void
DestroyStatusWorker(CiiClientRegistration_t *Client)
{
	//----------------------------------------------
	CiiMessage *Message;
	//----------------------------------------------

    ASSERT(Client->Signature == CII_BACKEND_SIGNATURE);

    //
    //  LOCK ----------------------------------------------------------------
    //
    pthread_mutex_lock(&ClientListLock);

    ClientList.remove(Client);

    LogMessage("CiiStat: Client Removed.  Current Total: %d", ClientList.size());

    //
    //  LOCK ----------------------------------------------------------------
    //
    pthread_mutex_lock(&Client->Status.Lock);

    //
    //  Empty the status message queue.
    //
    while (!Client->Status.Messages->empty()) {

        Message = Client->Status.Messages->back();
        Client->Status.Messages->pop_back();

        ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

        CiiFreeMessage(Message);
    }

    //
    //  Thread should tear itself down.
    //
    Client->Status.TerminateThread = true;

    //
    //  UNLOCK --------------------------------------------------------------
    //
    pthread_mutex_unlock(&Client->Status.Lock);

    //
    //  UNLOCK --------------------------------------------------------------
    //
    pthread_mutex_unlock(&ClientListLock);


    sem_post(&Client->Status.ThreadMessageSemaphore);
}


