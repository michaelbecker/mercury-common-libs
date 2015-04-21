#ifndef CII_PRIVATE_H__
#define CII_PRIVATE_H__

#include <pthread.h>
#include <semaphore.h>

using namespace std;

#define CII_MESSAGE_STRUCT_SIGNATURE 'CMSG'
#define CII_BACKEND_SIGNATURE   'BACK'



/**
 *  Backend structure needed to register with CII in order that
 *  status messages will be sent to you to send out.
 */
typedef struct CiiClientRegistration_t_{

    unsigned long Signature;

    void *ClientData;
    SendDataFcn_t     SendDataCallback;
    CloseBackendFcn_t CloseBackend;
    IncrementReferenceFcn_t IncBackendRef;
    DecrementReferenceFcn_t DecBackendRef;
    

    struct {

        list<CiiMessage *> *Messages;
        pthread_mutex_t Lock;
        pthread_t   ThreadId;
        sem_t  ThreadMessageSemaphore;
        bool TerminateThread;

    } Status;

    AccessLevel GrantedAccess;
    Login_t Login;

}CiiClientRegistration_t;


//
//  Bitmap for each callback.
//
enum MessageCallbackFlags{
    McfDeprecated = 0x1,    // Tells the CII to send a deprecated error code.
};

typedef struct MessageCallbackContainer_t_ {
    
    MessageCallback_t Callback;     //  The actual function pointer.
    void *UserData;                 //  The UserData that gets passed into the above.
    unsigned int Flags;

}MessageCallbackContainer_t;



CiiMessage * 
CiiCopyMessage(CiiMessage *Original);


void
CreateStatusWorker(CiiClientRegistration_t *Client);

void
DestroyStatusWorker(CiiClientRegistration_t *Client);


void
LogLogin(CiiClientRegistration_t *Client);

void
LogDisconnect(CiiClientRegistration_t *Client);


extern pthread_mutex_t ClientListLock;


#endif 
