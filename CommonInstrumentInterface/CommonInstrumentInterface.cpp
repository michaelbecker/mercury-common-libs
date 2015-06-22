#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdint.h>

#include <map>
#include <list>

#include "TaWinAssert.h"
#include "CommonInstrumentInterface.h"
#include "CiiBackend.h"
#include "CommFaultInjectionPrivate.h"
#include "TriosSocketAccept.h"
#include "LogLib.h"
#include "WorkQueue.h"
#include "CiiPrivate.h"
#include "Version.h"


/**@{*/
/**
 *  Lists of the various kinds of commands that we listen for.
 *  You need to be holding ApplicationCallbackLock to access these
 *	data elements.
 */
static map<unsigned int, MessageCallbackContainer_t> RegisteredGetMessages;
static map<unsigned int, MessageCallbackContainer_t> RegisteredActionMessages;
/**@}*/


/**
 *  Is there an active Master connection?  We cache it since it's easy to do.
 */
static bool IsMasterSet = false;
static CiiClientRegistration_t *MasterClient = NULL;
static pthread_mutex_t BackendListLock;

/**
 *  Count the Local UI Connections.
 */
static int LocalUICount = 0;


/**
 *  Global Module lock to serialize application callbacks, so that 
 *  the App doesn't have to worry that two or more callbacks are 
 *  being called simultaneously.  
 *  @todo - A future enhancement might be to convert this to a reader / writer
 *  lock and take a reader lock on GETs and a writer on SETs and ACTIONs.
 *	
 *	UPDATE - going to use this for locking all Callback lists too.  Accessing 
 *			 RegisteredGetMessages & RegisteredActionMessages will require 
 *			 this lock.
 */
static pthread_mutex_t ApplicationCallbackLock;


enum MessageState {

    MsPostCallback      = 0x1,
    MsMessageComplete   = 0x2
};



/**
 *  API to get the currently connected Master.
 */
int __attribute__  ((visibility ("default")))
CiiGetActiveMaster(LoginInfo_t * CurMaster)
{
    //-------------
    int Success;
    //-------------

    //
    //  LOCK ----------------------------------------------------------------
    //
    pthread_mutex_lock(&BackendListLock);

    if (IsMasterSet && MasterClient){

        if (CurMaster){
            memcpy(CurMaster, &MasterClient->Login.Info, sizeof(LoginInfo_t));
        }

        Success = 1;
    }
    else{
        Success = 0;
    }

    pthread_mutex_unlock(&BackendListLock);
    //
    //  UNLOCK --------------------------------------------------------------
    //

    return Success;
}



/**
 *  API to get whether a LocalUI is connected or not.
 *  This returns the count of the Local UIs attached.
 */
int __attribute__  ((visibility ("default")))
CiiLocalUIActive(void)
{
    //-------------
    int Success;
    //-------------

    //
    //  LOCK ----------------------------------------------------------------
    //
    pthread_mutex_lock(&BackendListLock);

    Success = LocalUICount;

    pthread_mutex_unlock(&BackendListLock);
    //
    //  UNLOCK --------------------------------------------------------------
    //

    return Success;
}



/**
 *  Add a Get command to listen for.
 */
void __attribute__  ((visibility ("default")))
CiiRegisterGetCommandMessage(   unsigned int Command, 
                                MessageCallback_t Callback, 
                                void *UserData
                                )
{
    //---------------------------------
    MessageCallbackContainer_t c;
    //---------------------------------

    ASSERT( IS_GET_MSG(Command) );

    c.Callback = Callback;
    c.UserData = UserData;
    c.Flags = 0;

	//
	//  LOCK ----------------------------------------------------
	//
	pthread_mutex_lock(&ApplicationCallbackLock);

    if (RegisteredGetMessages.find(Command) != RegisteredGetMessages.end()){
        LogMessage("DOUBLE REGISTERED GET COMMAND 0x%08x", Command);
        sleep(5);
        abort();
    }

    RegisteredGetMessages[Command] = c;

	//
	//  UNLOCK --------------------------------------------------
	//
	pthread_mutex_unlock(&ApplicationCallbackLock);
}



/**
 *  Add an Action command to listen for.
 */
void __attribute__  ((visibility ("default")))
CiiRegisterActionCommandMessage(    unsigned int Command, 
                                    MessageCallback_t Callback,
                                    void *UserData
                                    )
{
    //---------------------------------
    MessageCallbackContainer_t c;
    //---------------------------------

    ASSERT( IS_ACTION_MSG(Command) );
    
    c.Callback = Callback;
    c.UserData = UserData;
    c.Flags = 0;

	//
	//  LOCK ----------------------------------------------------
	//
	pthread_mutex_lock(&ApplicationCallbackLock);
    
    if (RegisteredActionMessages.find(Command) != RegisteredActionMessages.end()){
        LogMessage("DOUBLE REGISTERED ACTION COMMAND 0x%08x", Command);
        sleep(5);
        abort();
    }
	
    RegisteredActionMessages[Command] = c;

	//
	//  UNLOCK --------------------------------------------------
	//
	pthread_mutex_unlock(&ApplicationCallbackLock);
}



/**
 *  Deprecate a Get command
 */
void __attribute__  ((visibility ("default")))
CiiDeprecateGetCommandMessage(  unsigned int Command
                                )
{
    //---------------------------------
    MessageCallbackContainer_t c;
    //---------------------------------

    ASSERT( IS_GET_MSG(Command) );

    c.Callback = NULL;
    c.UserData = NULL;
    c.Flags = McfDeprecated;

	//
	//  LOCK ----------------------------------------------------
	//
	pthread_mutex_lock(&ApplicationCallbackLock);

    if (RegisteredGetMessages.find(Command) != RegisteredGetMessages.end()){
        LogMessage("DOUBLE REGISTERED GET COMMAND 0x%08x", Command);
        sleep(5);
        abort();
    }

    RegisteredGetMessages[Command] = c;

	//
	//  UNLOCK --------------------------------------------------
	//
	pthread_mutex_unlock(&ApplicationCallbackLock);
}



/**
 *  Deprecate an Action command
 */
void __attribute__  ((visibility ("default")))
CiiDeprecateActionCommandMessage(   unsigned int Command
                                    )
{
    //---------------------------------
    MessageCallbackContainer_t c;
    //---------------------------------

    ASSERT( IS_ACTION_MSG(Command) );
    
    c.Callback = NULL;
    c.UserData = NULL;
    c.Flags = McfDeprecated;

	//
	//  LOCK ----------------------------------------------------
	//
	pthread_mutex_lock(&ApplicationCallbackLock);

    if (RegisteredActionMessages.find(Command) != RegisteredActionMessages.end()){
        LogMessage("DOUBLE REGISTERED ACTION COMMAND 0x%08x", Command);
        sleep(5);
        abort();
    }

	RegisteredActionMessages[Command] = c;

	//
	//  UNLOCK --------------------------------------------------
	//
	pthread_mutex_unlock(&ApplicationCallbackLock);
}



/**
 *  Helper function.  An application will call this to add Response 
 *  data to a message in preperation to send it.
 *  
 *  YOU CAN ONLY CALL THIS ONCE!  MULTIPLE CALLS ON THE SAME 
 *  MESSAGE WILL ASSERT.
 */ 
void __attribute__  ((visibility ("default")))
CiiAddOptionalResponseData(CII_MESSAGE_HANDLE Msg, unsigned char *Data, unsigned int DataLength)
{
    CiiMessage *Message = (CiiMessage *)Msg;
    ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

    ASSERT(DataLength > 0);
    ASSERT(Data != NULL);

    ASSERT(Message->ResponseData == NULL);
    Message->ResponseData = (CiiMessageData *)malloc(RESPONSE_HEADER_SIZE + DataLength);
    ASSERT(Message->ResponseData);

    Message->ResponseDataLength = DataLength;
    Message->ResponseData->Type = MtResponse;
    memcpy(Message->ResponseData->Rsp.Data, Data, DataLength);

    switch(Message->MessageData->Type){

        case MtGetCommand:
            Message->ResponseData->Rsp.SequenceNumber = Message->MessageData->Get.SequenceNumber;
            Message->ResponseData->Rsp.SubCommand = Message->MessageData->Get.SubCommand;
            break;

        case MtActionCommand:
            Message->ResponseData->Rsp.SequenceNumber = Message->MessageData->Action.SequenceNumber;
            Message->ResponseData->Rsp.SubCommand = Message->MessageData->Action.SubCommand;
            break;

        default:
            ASSERT(!"Should not be here!");
            break;
    }
}



/**
 *  Send an ACK out whatever comm channel this came in on.
 */
static void 
SendAck(CiiMessage *Message)
{
    //-----------------------
    CiiMessageData Ack;
    //-----------------------

    ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

    if (CfiAckDelay){
        usleep(CfiAckDelay * 1000);
    }

    Ack.Type = MtAck;

    switch(Message->MessageData->Type){

        case MtGetCommand:
            Ack.Ack.SequenceNumber = Message->MessageData->Get.SequenceNumber;
            break;

        case MtActionCommand:
            Ack.Ack.SequenceNumber = Message->MessageData->Action.SequenceNumber;
            break;

        default:
            ASSERT(!"Should not be here!");
            break;
    }

    Message->SendDataCallback( Message->ClientData, &Ack, ACK_SIZE );

    //
    //  Only bother if we are outside of the callback.
    //
    if (Message->Flags & MsPostCallback){
        sem_post(&Message->AckEvent);
    }
}



/**
 *  Send a NAK out whatever comm channel this came in on.
 */
static void 
SendNak(CiiMessage *Message, unsigned int ErrorCode)
{
    //-----------------------
    CiiMessageData Nak;
    //-----------------------

    ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

    if (CfiNakDelay){
        usleep(CfiNakDelay * 1000);
    }

    Nak.Type = MtNak;
    Nak.Nak.ErrorCode = ErrorCode;

    switch(Message->MessageData->Type){

        case MtGetCommand:
            Nak.Nak.SequenceNumber = Message->MessageData->Get.SequenceNumber;
            break;

        case MtActionCommand:
            Nak.Nak.SequenceNumber = Message->MessageData->Action.SequenceNumber;
            break;

        default:
            ASSERT(!"Should not be here!");
            break;
    }

    Message->SendDataCallback( Message->ClientData, &Nak, NAK_SIZE );

    CiiFreeMessage(Message);
}



/**
 *  Send a LOGIN ACCEPT out whatever comm channel this came in on.
 */
static void 
SendLoginAccept(CiiMessage *Message, unsigned int GrantedAccess)
{
    ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

    Message->MessageData->Type = MtAccept;
    Message->MessageData->LoginAccept.GrantedAccess = GrantedAccess;

    Message->SendDataCallback( Message->ClientData, Message->MessageData, ACCEPT_SIZE );

    CiiFreeMessage(Message);
}



/**
 *  Called by app to complete a message that they returned Pending to
 *  when their callback was called.
 */ 
void __attribute__  ((visibility ("default")))
CiiCompletePendingCommand(  CII_MESSAGE_HANDLE Msg, 
                            MsgStatus Status)
{
    //----------------------------------------
    CiiMessage *Message = (CiiMessage *)Msg;
    CiiMessageData SimpleResponse;
    //----------------------------------------

    ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

    //
    //  We are being called after we exited the callback, act normal.
    //
    if (Message->Flags & MsPostCallback){
        sem_wait(&Message->AckEvent);
    }
    //
    //  Else we are being called "within" the callback itself. 
    //  We will complete the entire comm transaction right now!
    //
    else{
        SendAck(Message);
    }

    if (Message->ResponseData){
        Message->ResponseData->Rsp.StatusCode = Status;
        Message->SendDataCallback(  Message->ClientData, 
                                    Message->ResponseData, 
                                    RESPONSE_HEADER_SIZE + Message->ResponseDataLength );
    }
    else{
        //
        //  We are piggybacking a Rsp on top of the Message itself, because 
        //  there isn't any Rsp data to send here.  We could play games with 
        //  the internals of the struct layout, but it's safer to just make 
        //  a copy and work from there, in case the internal data structure 
        //  layouts change.
        //
        SimpleResponse.Type = MtResponse;
        SimpleResponse.Rsp.StatusCode = Status;

        switch(Message->MessageData->Type){

            case MtGetCommand:
                SimpleResponse.Rsp.SequenceNumber = Message->MessageData->Get.SequenceNumber;
                SimpleResponse.Rsp.SubCommand = Message->MessageData->Get.SubCommand;
                break;

            case MtActionCommand:
                SimpleResponse.Rsp.SequenceNumber = Message->MessageData->Action.SequenceNumber;
                SimpleResponse.Rsp.SubCommand = Message->MessageData->Action.SubCommand;
                break;

            default:
                ASSERT(!"Should not be here!");
                break;
        }

        Message->SendDataCallback(  Message->ClientData, 
                                    &SimpleResponse, 
                                    RESPONSE_HEADER_SIZE );
    }

    //
    //  ONLY free this if we are post callback.
    //
    if (Message->Flags & MsPostCallback){
        CiiFreeMessage(Message);
    }
    //
    //  We are still "in the callback!" Don't free, instead mark it 
    //  for later deletion.
    //
    else{
        Message->Flags |= MsMessageComplete;
    }
}



/**
 *  Helper function to complete a comm sequence after a callback was made.
 */
static void 
HandleMessageCompletion(    CiiMessage *Message,
                            MsgStatus Status)
{
    //
    //  The Callback did all the work, just cleanup and leave.
    //
    if (Message->Flags & MsMessageComplete){
        CiiFreeMessage(Message);
        return;
    }

    //
    //  Everything happened synchronously, we can finish.
    //
    if (Status == MsSuccess){
        SendAck(Message);
        //
        //  This sends the response.
        //
        CiiCompletePendingCommand(Message, Status);
    }
    //
    //  The app has decided to hold onto the message.  We send 
    //  The ACK, but the App has to call CiiCompletePendingCommand();
    //
    else if (Status == MsPending){
        SendAck(Message);
    }
    //
    //  It failed for whatever reason.
    //
    else{
        SendNak(Message, Status);
    }
}



/**
 *  Generic routine to receive messages and send them to the app.
 *  Expected to be called from multiple specific comm channels.
 */
void __attribute__  ((visibility ("default")))
CiiBackendRouteReceivedMessage(CII_BACK_END_HANDLE ClientHandle, CiiMessage *Message)
{
    //-------------------------------------------------------------------
    map<unsigned int, MessageCallbackContainer_t>::iterator AppCallback;
    MsgStatus Status;
    MessageCallback_t Callback = NULL;
    void *UserData;
    int MessageCallbackFlags;
    Login_t *Login;
    unsigned int LoginLength;
    CiiClientRegistration_t *Client;
    int rc = -1;
    //-------------------------------------------------------------------

    Client = (CiiClientRegistration_t *)ClientHandle;

    ASSERT(Client->Signature == CII_BACKEND_SIGNATURE);
    ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);


    switch(Message->MessageData->Type){
        
        //
        //  -----------------------------------------------------------------
        //
        case MtGetCommand:

            if (Client->GrantedAccess == AlInvalid){
                SendNak(Message, MsNotLoggedIn);
                return;
            }

            if (Message->MessageDataLength < GET_HEADER_SIZE){
                SendNak(Message, MsMalformedMessage);
                return;
            }

			//
			//  LOCK ----------------------------------------------------
			//
			pthread_mutex_lock(&ApplicationCallbackLock);

			AppCallback = RegisteredGetMessages.find(Message->MessageData->Get.SubCommand);

			//
			//  UNLOCK --------------------------------------------------
			//
			pthread_mutex_unlock(&ApplicationCallbackLock);


            if (AppCallback == RegisteredGetMessages.end()){
                LogMessage("Unknown Get Command: 0x%08x", Message->MessageData->Get.SubCommand);
                SendNak(Message, MsUnknownCommand);
                return;
            }

            Callback = AppCallback->second.Callback;
            UserData = AppCallback->second.UserData;
            MessageCallbackFlags = AppCallback->second.Flags;

            if (MessageCallbackFlags & McfDeprecated){
                SendNak(Message, MsDeprecated);
                return;
            }

            rc = sem_init(&Message->AckEvent, 0, 0);
            ASSERT(rc == 0);
            Message->AckEventInitialized = 1;

            Status = Callback(Message, UserData);

            Message->Flags |= MsPostCallback;

            HandleMessageCompletion(Message, Status);

            break;


        //
        //  -----------------------------------------------------------------
        //
        case MtActionCommand:

            if (Client->GrantedAccess == AlInvalid){
                SendNak(Message, MsNotLoggedIn);
                return;
            }

            if (Client->GrantedAccess < AlMaster){
                SendNak(Message, MsAccessDenied);
                return;
            }

            if (Message->MessageDataLength < ACTION_HEADER_SIZE){
                SendNak(Message, MsMalformedMessage);
                return;
            }

			//
			//  LOCK ----------------------------------------------------
			//
			pthread_mutex_lock(&ApplicationCallbackLock);

			AppCallback = RegisteredActionMessages.find(Message->MessageData->Action.SubCommand);

			//
			//  UNLOCK --------------------------------------------------
			//
			pthread_mutex_unlock(&ApplicationCallbackLock);


            if (AppCallback == RegisteredActionMessages.end()){
                LogMessage("Unknown Action Command: 0x%08x", Message->MessageData->Action.SubCommand);
                SendNak(Message, MsUnknownCommand);
                return;
            }

            Callback = AppCallback->second.Callback;
            UserData = AppCallback->second.UserData;
            MessageCallbackFlags = AppCallback->second.Flags;

            if (MessageCallbackFlags & McfDeprecated){
                SendNak(Message, MsDeprecated);
                return;
            }

            rc = sem_init(&Message->AckEvent, 0, 0);
            ASSERT(rc == 0);
            Message->AckEventInitialized = 1;

            Status = Callback(Message, UserData);

            Message->Flags |= MsPostCallback;

            HandleMessageCompletion(Message, Status);

            break;

        //
        //  -----------------------------------------------------------------
        //
        case MtLogin:
            //
            //  Only get to do it once.
            //
            if (Client->GrantedAccess != AlInvalid){
                Message->MessageData->Nak.SequenceNumber = 0xFFFFFFFF;
                SendNak(Message, MsAccessDenied);
                return;
            }

            Login = (Login_t *)LOGIN_DATA(Message);
            LoginLength = LOGIN_DATA_LENGTH(Message);

            //
            //  Change, we will be permissive of extra data, and just ignore it.
            //
            if (LoginLength < sizeof(Login_t)){
                Message->MessageData->Nak.SequenceNumber = 0xFFFFFFFF;
                SendNak(Message, MsMalformedMessage);
                return;
            }

            //
            //  LOCK ----------------------------------------------------------------
            //
            pthread_mutex_lock(&BackendListLock);

            switch (Login->RequestedAccess){

                case AlViewOnly:
                    Client->GrantedAccess = AlViewOnly;
                    break;

                case AlMaster:
                    if (IsMasterSet){
                        Client->GrantedAccess = AlViewOnly;
                    }
                    else{
                        Client->GrantedAccess = AlMaster;
                        MasterClient = Client;
                        IsMasterSet = true;
                    }
                    break;

                case AlLocalUI:
                    Client->GrantedAccess = AlLocalUI;
                    LocalUICount++;
                    break;

                case AlEngineering:
                    Client->GrantedAccess = AlEngineering;
                    break;

                default:
                    //
                    //  UNLOCK --------------------------------------------------------------
                    //
                    pthread_mutex_unlock(&BackendListLock);
                    Message->MessageData->Nak.SequenceNumber = 0xFFFFFFFF;
                    SendNak(Message, MsMalformedMessage);
                    return;
            }


            memcpy(&Client->Login, Login, sizeof(Login_t));

            //
            //  UNLOCK --------------------------------------------------------------
            //
            pthread_mutex_unlock(&BackendListLock);

            LogLogin(Client);

            SendLoginAccept(Message, Client->GrantedAccess);

            break;


        //
        //  -----------------------------------------------------------------
        //
        default:
            LogMessage("Cii: Invalid Message type to receive!");
            return;
    }
}



void __attribute__  ((visibility ("default")))
CiiGetCommandData(  CII_MESSAGE_HANDLE Msg,     //  [in]  Handle to the message.
                    unsigned char **Data,       //  [out] Pointer to the data pointer.
                    unsigned int *DataLength    //  [out] Pointer to the data size.
                    )
{
    CiiMessage *Message = (CiiMessage *)Msg;
    ASSERT(Message->Signature == CII_MESSAGE_STRUCT_SIGNATURE);

    switch(Message->MessageData->Type){

        case MtGetCommand:
            *Data = GET_DATA(Message);
            *DataLength = GET_DATA_LENGTH(Message);
            break;

        case MtActionCommand:
            *Data = ACTION_DATA(Message);
            *DataLength = ACTION_DATA_LENGTH(Message);
            break;

        default:
            *Data = NULL;
            *DataLength = 0;
            break;
    }
}



list<CiiClientRegistration_t *>BackendList;



CII_BACK_END_HANDLE __attribute__  ((visibility ("default")))
CiiRegisterBackend( void *ClientData, 
                SendDataFcn_t SendDataCallback, 
                CloseBackendFcn_t CloseBackend,
                IncrementReferenceFcn_t IncBackendRef,
                DecrementReferenceFcn_t DecBackendRef
                )
{
    //----------------------------------------------
    CiiClientRegistration_t *Client;
    //----------------------------------------------

    Client = (CiiClientRegistration_t *)malloc(sizeof(CiiClientRegistration_t));
    ASSERT(Client != NULL);

    memset(Client, 0, sizeof(CiiClientRegistration_t));

    Client->Signature           = CII_BACKEND_SIGNATURE;
    Client->ClientData          = ClientData;
    Client->SendDataCallback    = SendDataCallback;
    Client->CloseBackend        = CloseBackend;
    Client->IncBackendRef       = IncBackendRef;
    Client->DecBackendRef       = DecBackendRef;

    CreateStatusWorker(Client);

    pthread_mutex_lock(&BackendListLock);
    BackendList.push_back(Client);
    pthread_mutex_unlock(&BackendListLock);

    return (CII_BACK_END_HANDLE)Client;
}



void __attribute__  ((visibility ("default")))
CiiUnregisterBackend(CII_BACK_END_HANDLE ClientHandle)
{
    //----------------------------------------------
    CiiClientRegistration_t *Client;
    //----------------------------------------------

    Client = (CiiClientRegistration_t *)ClientHandle;
	ASSERT(Client);
    ASSERT(Client->Signature == CII_BACKEND_SIGNATURE);
    
    pthread_mutex_lock(&BackendListLock);

    if (Client->GrantedAccess == AlMaster){
        IsMasterSet = false;
        MasterClient = NULL;
    }
    else if (Client->GrantedAccess == AlLocalUI){
        LocalUICount--;
    }

    BackendList.remove(Client);

    pthread_mutex_unlock(&BackendListLock);

    DestroyStatusWorker(Client);
}



DbgClientConnection_t __attribute__  ((visibility ("default"))) * 
CiiDbgGetClientList(unsigned int *Count)
{
    DbgClientConnection_t *Clients = NULL;

    pthread_mutex_lock(&BackendListLock);

    unsigned int NumConnections = BackendList.size();

    *Count = NumConnections;

    if (NumConnections){

        Clients = (DbgClientConnection_t *)malloc(sizeof(DbgClientConnection_t) * NumConnections);
        ASSERT(Clients);

        list<CiiClientRegistration_t *>::iterator it = BackendList.begin();
        unsigned int i = 0;

        for ( ; it != BackendList.end() ; it++){

            CiiClientRegistration_t *Client = *it;

            Clients[i].ConnectionHandle = (uintptr_t)Client;
            Clients[i].GrantedAccess = Client->GrantedAccess;
            memcpy(&Clients[i].Login, &Client->Login, sizeof(Login_t));
            i++; 
        }
    }

    pthread_mutex_unlock(&BackendListLock);

    return Clients;
}



void __attribute__  ((visibility ("default")))
CiiDbgDropClient(unsigned int Handle)
{
    //-------------------------------------
    uintptr_t CorrectSizedHandle = Handle; 
    CiiClientRegistration_t *Client;
    bool FoundClient = false;
    //-------------------------------------
    
    Client = (CiiClientRegistration_t *)CorrectSizedHandle;

    if ((CorrectSizedHandle == 0) || (CorrectSizedHandle >= 0x80000000)){
        return;
    }

    //
    //  LOCK -------------------------------------------
    //
    pthread_mutex_lock(&BackendListLock);

    list<CiiClientRegistration_t *>::iterator it = BackendList.begin();

    for ( ; it != BackendList.end(); it++){

        //
        //  Relying on operator short circuit here.
        //
        if ((*it == Client) && (Client->Signature == CII_BACKEND_SIGNATURE)){

            FoundClient = true;
            break;
        }
    }

    //
    //  UNLOCK -----------------------------------------
    //
    pthread_mutex_unlock(&BackendListLock);


    if (FoundClient){
        Client->CloseBackend(Client->ClientData);
    }

}



extern WORK_QUEUE_HANDLE StatusWorkQueue;
extern void InitSocketManagerStats(void);
extern void InitLogSocketTrace(void);


void __attribute__  ((visibility ("default"))) 
CiiInitializeLibrary(void)
{
	StatusWorkQueue = CreateWorkQueue();

    InitSocketManagerStats();

    InitLogSocketTrace();

    //
    //  Start Trios comm, and we are done.
    //
    InitTriosCommunications();
}



void __attribute__  ((visibility ("default"))) 
CiiPreInitLibrary(void)
{
    //----------------------------
    pthread_mutexattr_t attr;
    int rc;
    //----------------------------

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&BackendListLock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutex_init(&ClientListLock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutex_init(&ApplicationCallbackLock, &attr);
    ASSERT(rc == 0);
    
    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);

    LOG_LIBRARY_VERSION();
}


