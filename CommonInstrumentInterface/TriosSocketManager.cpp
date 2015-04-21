#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>


#include "TaWinAssert.h"
#include "CommonInstrumentInterface.h"
#include "CiiBackend.h"
#include "CommFaultInjectionPrivate.h"
#include "ConsoleMemoryDump.h"
#include "LogLib.h"
#include "TriosSocketManager.h"

// #include "CommonFiles.h" ??? - why here?



static struct SocketManagerStats_t {

    pthread_mutex_t Lock;

    int CurSocketThreads;
    int MaxSocketThreads;
    int CurSocketManagerControlStructs;

}SocketManagerStats;



/**
 *  Sanity check, right now not planning on Trios sending us 
 *  anything greater than 100 MB, probably should be much less.
 */
#define MAX_SOCKET_LENGTH 100*1024*1024


//
//  The SocketHeader and SocketFooter need to be packed.
//
#pragma pack(1)
/**
 *  Standard lightweight header for TCP socket communication to 
 *  find message boundaries.
 */
typedef struct SocketHeader_ {

    char Sync[4];
    unsigned int Length;

}SocketHeader;

/**
 *  Standard lightweight footer for TCP socket communication to 
 *  find message boundaries.
 */
typedef struct SocketFooter_ {

    char End[4];

}SocketFooter;

#pragma pack()


#define SOCKET_MANAGER_SIGNATURE    'SOCK'
/**
 *  Structure to encapsulate everything needed to send messages. 
 */
struct SocketManager_t {

    unsigned int     Signature;
    pthread_mutex_t  SendLock;
    int              ClientSocket;
    unsigned int     ReferenceCount;
    pthread_t        ThreadId;
};



void
InitSocketManagerStats(void)
{
    //----------------------------
    pthread_mutexattr_t attr;
    int rc;
    //----------------------------

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&SocketManagerStats.Lock, &attr);
    ASSERT(rc == 0);
    
    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);


    memset(&SocketManagerStats, 0, sizeof(SocketManagerStats));
}



//
//  You need to be holding the stats lock here.
//
static void
LogSocketManagerStatistics(void)
{
    LogMessage("SockMgr: CurThreads:     %d", SocketManagerStats.CurSocketThreads);
    LogMessage("SockMgr: MaxThreads:     %d", SocketManagerStats.MaxSocketThreads);
    LogMessage("SockMgr: CurCtrlStructs: %d", SocketManagerStats.CurSocketManagerControlStructs);
}



//
//  Socket Trace stuff ------------------------------------
//
typedef struct SocketTraceHeader_t_{
    
    int IsTx;               //  Boolean
    struct timespec Time;   //  Time written
    unsigned int Id;        //  Socket Mgr Thread ID
    int Length;             //  Length of Data

    //
    //  Data Follows...
    //
}SocketTraceHeader_t;



static volatile bool SocketTraceEnabled = false;
static int hLogSocketTraffic = -1;
static pthread_mutex_t SocketTraceLock;



void
LogSocketTrafficEx(int IsTx, unsigned int Id, unsigned char *Data, int Length)
{
    SocketTraceHeader_t TraceHeader;
    int BytesWritten;

    TraceHeader.IsTx = IsTx;
    TraceHeader.Id = Id;
    TraceHeader.Length = Length;

    clock_gettime(CLOCK_REALTIME, &TraceHeader.Time);

    pthread_mutex_lock(&SocketTraceLock);

    BytesWritten = write(hLogSocketTraffic, &TraceHeader, sizeof(TraceHeader));
    ASSERT(BytesWritten == sizeof(TraceHeader));

    BytesWritten = write(hLogSocketTraffic, Data, Length);
    ASSERT(BytesWritten == Length);
    
    pthread_mutex_unlock(&SocketTraceLock);
}



#define LogSocketTraffic(IsTx_, Id_, Data_, Length_)        \
    {                                                       \
        if (SocketTraceEnabled) {                           \
            LogSocketTrafficEx(IsTx_, Id_, Data_, Length_); \
        }                                                   \
    }



void
InitLogSocketTrace(void)
{
    //----------------------------
    pthread_mutexattr_t attr;
    int rc;
    //----------------------------

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&SocketTraceLock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);
}


#define CII_SOCKET_TRACE_FILE "CiiSocketTrace.dat"


void __attribute__  ((visibility ("default")))
CiiDbgEnableSocketTrace(int Enable)
{
    pthread_mutex_lock(&SocketTraceLock);

    if (Enable && !SocketTraceEnabled){
        SocketTraceEnabled = true;
        hLogSocketTraffic = creat(CII_SOCKET_TRACE_FILE, 0644);
        ASSERT(hLogSocketTraffic > 0);
    }
    else if (!Enable && SocketTraceEnabled){
        SocketTraceEnabled = false;
        close(hLogSocketTraffic);
    }

    pthread_mutex_unlock(&SocketTraceLock);
}
//
//  END Socket Trace stuff --------------------------------
//



static void
LogSocketManagerCreation(SocketManager_t *SocketManager)
{
    ASSERT(SocketManager->Signature == SOCKET_MANAGER_SIGNATURE);

    //
    //  LOCK --------------------------------------------
    //
    pthread_mutex_lock(&SocketManagerStats.Lock);

    LogMessage("SockMgr[%p]: New client connected", 
                SocketManager->ThreadId);

    SocketManagerStats.CurSocketThreads++;
    SocketManagerStats.CurSocketManagerControlStructs++;

    if (SocketManagerStats.CurSocketThreads > SocketManagerStats.MaxSocketThreads){
        SocketManagerStats.MaxSocketThreads = SocketManagerStats.CurSocketThreads;
    }

    LogSocketManagerStatistics();

    //
    //  UNLOCK -----------------------------------------
    //
    pthread_mutex_unlock(&SocketManagerStats.Lock);
}



static void
LogSocketManagerThreadTeardown(SocketManager_t *SocketManager)
{
    ASSERT(SocketManager->Signature == SOCKET_MANAGER_SIGNATURE);
    
    //
    //  LOCK -------------------------------------------
    //
    pthread_mutex_lock(&SocketManagerStats.Lock);

    LogMessage("SockMgr[%p]: Tearing down connection - RefCount: %d", 
                SocketManager->ThreadId, 
                SocketManager->ReferenceCount);

    SocketManagerStats.CurSocketThreads--;

    LogSocketManagerStatistics();

    //
    //  UNLOCK ------------------------------------------
    //
    pthread_mutex_unlock(&SocketManagerStats.Lock);
}



static void
LogSocketManagerFree(SocketManager_t *SocketManager)
{
    ASSERT(SocketManager->Signature == SOCKET_MANAGER_SIGNATURE);

    //
    //  LOCK    --------------------------------------
    //
    pthread_mutex_lock(&SocketManagerStats.Lock);

    LogMessage("SockMgr[%p]: Final free of connection", 
                SocketManager->ThreadId);

    SocketManagerStats.CurSocketManagerControlStructs--;

    LogSocketManagerStatistics();

    //
    //  UNLOCK  ----------------------------------------
    //
    pthread_mutex_unlock(&SocketManagerStats.Lock);
}



/**
 *  Allocate a SocketManager_t.  This is a reference counted struct.
 */
static SocketManager_t *
AllocateSocketManagerControl(int ClientSocket)
{
    //----------------------------
    pthread_mutexattr_t attr;
    int rc;
    //----------------------------
    
    SocketManager_t *sm = (SocketManager_t *)malloc(sizeof(SocketManager_t));
    ASSERT(sm != NULL);
    memset(sm, 0, sizeof(SocketManager_t));
    sm->Signature = SOCKET_MANAGER_SIGNATURE;

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&sm->SendLock, &attr);
    ASSERT(rc == 0);
    
    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);

    sm->ClientSocket = ClientSocket;
    sm->ReferenceCount = 1;

    return sm;
}



/**
 *  When the reference count hits zero, we clean up.
 */
static void 
DecrementSocketManagerReference(void *ClientData)
{
    SocketManager_t *sm = (SocketManager_t *)ClientData;

    ASSERT(sm->Signature == SOCKET_MANAGER_SIGNATURE);

    //
    //  LOCK ----------------------------------------------------------------
    //
    pthread_mutex_lock(&sm->SendLock);

    sm->ReferenceCount--;

    if (sm->ReferenceCount){
        //
        //  UNLOCK ----------------------------------------------------------
        //
        pthread_mutex_unlock(&sm->SendLock);
    }
    else{
        //
        //  UNLOCK ----------------------------------------------------------
        //
        pthread_mutex_unlock(&sm->SendLock);

        pthread_mutex_destroy(&sm->SendLock);

        LogSocketManagerFree(sm);
        
        //
        //  If we have bugs or race conditions here, bring them out...
        //
        memset(sm, 0xEE, sizeof(SocketManager_t));

        free(sm);
    }
}



/**
 *  Take another reference out on the SocketManager struct.
 */
static void
IncrementSocketManagerReference(void *ClientData)
{
    SocketManager_t *sm = (SocketManager_t *)ClientData;

    ASSERT(sm->Signature == SOCKET_MANAGER_SIGNATURE);

    //
    //  LOCK ----------------------------------------------------------------
    //
    pthread_mutex_lock(&sm->SendLock);

    sm->ReferenceCount++;

    //
    //  UNLOCK --------------------------------------------------------------
    //
    pthread_mutex_unlock(&sm->SendLock);
}



static void
CloseClientConnection(SocketManager_t *SocketManager)
{
    //---------------
    int rc;
    //---------------

    ASSERT(SocketManager->Signature == SOCKET_MANAGER_SIGNATURE);

    if (SocketManager->ClientSocket != -1) {
        // 
        //  shutdown the connection 
        //
        rc = shutdown(SocketManager->ClientSocket, SHUT_RDWR);
        if (rc == -1) {
            LogMessage("SockMgr[%p]: shutdown failed with error: %d", 
            SocketManager->ThreadId, errno);
        }
        //
        //  And close the socket handle
        //
        close(SocketManager->ClientSocket);
        //
        //  Flag the Socket as invalid.
        //
        SocketManager->ClientSocket = -1;
    }
}



/**
 *  Helper function to keep reading the socket until we 
 *  get all the data we need, or handles errors.
 */ 
static int 
ReceiveUntilComplete(SocketManager_t *SocketManager, char *Buffer, int LengthToRead)
{
    //-------------------------
    int Count = 0;    
    int rc;
    int RecvLength;
    int ClientSocket;
    //-------------------------

    RecvLength = LengthToRead;
    ClientSocket = SocketManager->ClientSocket;

    do {
        rc = recv(ClientSocket, Buffer, RecvLength, 0);

        if (rc == 0){
            LogMessage("SockMgr[%p]: Remote client closed the connection, cleaning up.",
                SocketManager->ThreadId );
            return 1;
        }
        else if (rc < 0){
            LogMessage("SockMgr[%p]: recv failed with error: %d", 
                SocketManager->ThreadId, errno);
            return 1;
        }

        Count += rc;
        Buffer += rc;
        RecvLength -= rc;

    } while(Count < LengthToRead);

    return 0;
}



/**
 *  This code doesn't know type or which buffer to use, which is 
 *  why we pass in a MessageData pointer and a length.
 */
static void 
SendTriosComm(void *ClientData, CiiMessageData *GenericData, unsigned int Length)
{
    //----------------------------------
    int rc;
    SocketHeader Header = {
        {'S', 'Y', 'N', 'C',},
        Length
    };
    SocketFooter Footer = {
        {'E', 'N', 'D', ' '}
    };
    SocketManager_t *SocketManager;
    //----------------------------------
    
    if (CfiCorruptSync()){
        Header.Sync[3] = 'X';
    }
    if (CfiCorruptLengthField()){
        Header.Length += CfiCorruptLengthFieldValue();
    }
    int CurrentCfiCorruptLength = CfiCorruptLength();
    if (CurrentCfiCorruptLength){
        Header.Length += CurrentCfiCorruptLength;
        if (Header.Length < 0)
            Header.Length = 0;
    }

    SocketManager = (SocketManager_t *)ClientData;
    ASSERT(SocketManager->Signature == SOCKET_MANAGER_SIGNATURE);

    //
    //  LOCK ----------------------------------------------------------------
    //
    pthread_mutex_lock(&SocketManager->SendLock);

    rc = send( SocketManager->ClientSocket, (const char *)&Header, sizeof(Header), MSG_NOSIGNAL );
    if (rc == -1) {
        goto ERROR_EXIT;
    }
    LogSocketTraffic(1, SocketManager->ThreadId, (unsigned char *)&Header, sizeof(Header));
    //ConsoleMemoryDump("Sending Header", (unsigned char *)&Header, sizeof(Header));


    ASSERT(GenericData != NULL);
    ASSERT(Length != 0);

    if (CurrentCfiCorruptLength < 0){
        rc = send( SocketManager->ClientSocket, (const char *)GenericData, Length + CurrentCfiCorruptLength, MSG_NOSIGNAL );
    }
    else{
        rc = send( SocketManager->ClientSocket, (const char *)GenericData, Length, MSG_NOSIGNAL );
    }
    if (rc == -1) {
        goto ERROR_EXIT;
    }
    //ConsoleMemoryDump("Sending Data", (unsigned char *)GenericData, Length);
    LogSocketTraffic(1, SocketManager->ThreadId, (unsigned char *)GenericData, Length);

    //
    //  Add garbage at the end if we are injecting faults.
    //
    if (CurrentCfiCorruptLength > 0){
        int cfiBufferLen = CurrentCfiCorruptLength;
        char *cfiBuffer = (char *)malloc(cfiBufferLen);
        CfiGetExtraRandomData((unsigned char *)cfiBuffer, cfiBufferLen);
        (void)send( SocketManager->ClientSocket, (const char *)cfiBuffer, cfiBufferLen, MSG_NOSIGNAL );
        //ConsoleMemoryDump("Sending Fault Injection Garbage Data", (unsigned char *)cfiBuffer, cfiBufferLen);
        LogSocketTraffic(1, SocketManager->ThreadId, (unsigned char *)cfiBuffer, cfiBufferLen);
        free(cfiBuffer);
    }

    rc = send( SocketManager->ClientSocket, (const char *)&Footer, sizeof(Footer), MSG_NOSIGNAL );
    if (rc == -1) {
        goto ERROR_EXIT;
    }
    //ConsoleMemoryDump("Sending Footer", (unsigned char *)&Footer, sizeof(Footer));
    LogSocketTraffic(1, SocketManager->ThreadId, (unsigned char *)&Footer, sizeof(Footer));


    //
    //  UNLOCK --------------------------------------------------------------
    //
    pthread_mutex_unlock(&SocketManager->SendLock);
    
    return;


ERROR_EXIT:
    //
    //  UNLOCK ----------------------------------------------------------
    //
    pthread_mutex_unlock(&SocketManager->SendLock);

    LogMessage("SockMgr[%p]: send failed with error: %d", 
        SocketManager->ThreadId, errno);

    CloseClientConnection(SocketManager);

    return;
}



void __attribute__  ((visibility ("default")))
CloseSocketManager(void *ClientData)
{
    //----------------------------------
    SocketManager_t *SocketManager;
    //----------------------------------

    SocketManager = (SocketManager_t *)ClientData;
    ASSERT(SocketManager->Signature == SOCKET_MANAGER_SIGNATURE);

    LogMessage("SockMgr[%p]: Cii Requested Close", SocketManager->ThreadId);
    CloseClientConnection(SocketManager);
}



/**
 *  Actual worker / callback thread.
 */ 
void *
TriosSocketManagerThread(void * Parameter)
{
    //------------------------------------------------
    int rc;

    SocketHeader Header = {0};
    SocketFooter Footer;

    CiiMessage *Message = NULL;
    SocketManager_t *SocketManager;

    CII_BACK_END_HANDLE ClientRegistration;
    //------------------------------------------------

    SocketManager = (SocketManager_t *)Parameter;

    //
    //  We do not need cleanup.
    //
    pthread_detach(pthread_self());
    
    LogSocketManagerCreation(SocketManager);

    //
    //  Register with the CommonMessageInterface.
    //
    ClientRegistration = CiiRegisterBackend(    SocketManager,      //  Generic Client Data Ptr
                                                SendTriosComm,      //  Send function
                                                CloseSocketManager, //  Close Backend function
                                                IncrementSocketManagerReference,
                                                DecrementSocketManagerReference
                                                );

    // 
    //  Receive until the peer shuts down the connection
    //
    for (;;){
        //
        //  Read the Header
        //
        rc = ReceiveUntilComplete(  SocketManager, 
                                    (char *)&Header, 
                                    sizeof(Header));
        if (rc){
            goto EXIT;
        }

        //ConsoleMemoryDump("Read Header", (unsigned char *)&Header, sizeof(Header));
        LogSocketTraffic(0, SocketManager->ThreadId, (unsigned char *)&Header, sizeof(Header));

        if ((Header.Sync[0] != 'S') ||
            (Header.Sync[1] != 'Y') ||
            (Header.Sync[2] != 'N') ||
            (Header.Sync[3] != 'C')){

            LogMessage("SockMgr[%p]: Failed SYNC! %02x(%c) %02x(%c) %02x(%c) %02x(%c)", 
                SocketManager->ThreadId,
                Header.Sync[0], Header.Sync[0], Header.Sync[1], Header.Sync[1],
                Header.Sync[2], Header.Sync[2], Header.Sync[3], Header.Sync[3]);
            goto EXIT;
        }
        else if ((Header.Length < 4) || (Header.Length > MAX_SOCKET_LENGTH)){
            LogMessage("SockMgr[%p]: Failed Length! %u", 
                SocketManager->ThreadId,
                Header.Length);
            goto EXIT;
        }

        //
        //  Header good, read message.
        //
        Message = CiiAllocateMessage(Header.Length);
        if (!Message){
            LogMessage("SockMgr[%p]: CiiAllocateMessage(%d) Failed!", 
            SocketManager->ThreadId, Header.Length);
            goto EXIT;
        }

        rc = ReceiveUntilComplete(  SocketManager, 
                                    (char *)Message->MessageData, 
                                    Header.Length);
        if (rc){
            CiiFreeMessage(Message);
            goto EXIT;
        }

        //ConsoleMemoryDump("Reading Data", (unsigned char *)Message->MessageData, Header.Length);
        LogSocketTraffic(0, SocketManager->ThreadId, (unsigned char *)Message->MessageData, Header.Length);

        //
        //  Finish with the footer
        //
        rc = ReceiveUntilComplete(SocketManager, (char *)&Footer, sizeof(Footer));

        if (rc){
            CiiFreeMessage(Message);
            goto EXIT;
        }

        //ConsoleMemoryDump("Read Footer", (unsigned char *)&Footer, sizeof(Footer));
        LogSocketTraffic(0, SocketManager->ThreadId, (unsigned char *)&Footer, sizeof(Footer));

        if ((Footer.End[0] != 'E') ||
            (Footer.End[1] != 'N') ||
            (Footer.End[2] != 'D') ||
            (Footer.End[3] != ' ')){

            LogMessage("SockMgr[%p]: Failed END! %02x(%c) %02x(%c) %02x(%c) %02x(%c)", 
                SocketManager->ThreadId,
                Footer.End[0], Footer.End[0], Footer.End[1], Footer.End[1],
                Footer.End[2], Footer.End[2], Footer.End[3], Footer.End[3]);

            CiiFreeMessage(Message);
            goto EXIT;
        }

        //
        //  The footer is good, we can service the message
        //
        Message->ClientData = SocketManager;
        Message->CleanupMessageCallback = DecrementSocketManagerReference;
        Message->SendDataCallback = SendTriosComm;
        IncrementSocketManagerReference(SocketManager);

        CiiBackendRouteReceivedMessage(ClientRegistration, Message);

    } // for (;;)


//
//  For whatever, we are tearing down the SocketManager and 
//  UI Connection.
//
EXIT:
    CloseClientConnection(SocketManager);

    CiiUnregisterBackend(ClientRegistration);

    LogSocketManagerThreadTeardown(SocketManager);

    DecrementSocketManagerReference(SocketManager);

    return 0;
}



int
CreateSocketManager(int ClientSocket)
{
    SocketManager_t *SocketManager = AllocateSocketManagerControl(ClientSocket);

    int rc = pthread_create(&SocketManager->ThreadId, NULL, TriosSocketManagerThread, SocketManager);
    ASSERT(rc == 0);
    
    return 0;
}


