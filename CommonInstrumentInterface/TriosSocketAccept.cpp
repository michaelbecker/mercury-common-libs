#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#include "TaWinAssert.h"
#include "TriosSocketAccept.h"
#include "TriosSocketManager.h"


//
//  Should be between  49152–65535
//
#define TRIOS_CONNECT_PORT "8080"  // Must be string!



//
//  Handles accepting connections from Trios.
//
void *
TriosSocketAcceptThread(void *Parameter)
{
    //----------------------------------------
    int rc;

    int ListenSocket = -1;
    int ClientSocket = -1;
    struct addrinfo hints;
    struct addrinfo *res;
    //----------------------------------------

    (void)Parameter;
 
    //
    //  We do not need cleanup.
    //
    pthread_detach(pthread_self());

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, TRIOS_CONNECT_PORT, &hints, &res);

    //
    //  Create a SOCKET for connecting to server.
    //  We need to add the SOCK_CLOEXEC or there are problems with 
    //  various upgrades because the new process we fork() / exec() 
    //  keeps the fd open.
    //
    ListenSocket = socket(res->ai_family, res->ai_socktype | SOCK_CLOEXEC, res->ai_protocol);
    if (ListenSocket == -1) {
        LogMessage("socket failed with error: %ld - %s", errno, strerror(errno));
        freeaddrinfo(res);
        sleep(5);
        abort();
        return NULL;
    }

    int yes=1;
    //char yes='1'; // Solaris people use this
    if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        LogMessage("setsockopt failed with error: %ld - %s", errno, strerror(errno));
        freeaddrinfo(res);
        close(ListenSocket);
        sleep(5);
        abort();
        return NULL;
    }


    // Setup the TCP listening socket
    rc = bind( ListenSocket, res->ai_addr, (int)res->ai_addrlen);
    if (rc == -1) {
        LogMessage("bind failed with error: %d - %s", errno, strerror(errno));
        freeaddrinfo(res);
        close(ListenSocket);
        sleep(5);
        abort();
        return NULL;
    }

    freeaddrinfo(res);

    rc = listen(ListenSocket, SOMAXCONN);
    if (rc == -1) {
        LogMessage("listen failed with error: %d - %s", errno, strerror(errno));
        close(ListenSocket);
        sleep(5);
        abort();
        return NULL;
    }

    
    for (;;){

        // 
        //  Accept a client socket - This is the blocking call!
        //
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == -1) {
            LogMessage("accept failed with error: %d - %s", errno, strerror(errno));
            close(ListenSocket);
            sleep(5);
            abort();
            return NULL;
        }

        //
        //  Create a worker thread...
        //
        rc = CreateSocketManager(ClientSocket);
        if (rc != 0){
            LogMessage("CreateSocketManager() failed");
            close(ListenSocket);
            sleep(5);
            abort();
            return NULL;
        }
    }
}



//
//  Kicks off all of Trios comm.
//
int 
InitTriosCommunications(void)
{
    pthread_t ThreadId;

    //
    //  Create the Socket Accept thread
    //
    int rc = pthread_create(&ThreadId, NULL, TriosSocketAcceptThread, NULL);
    ASSERT(rc == 0);


    return 0;
}


