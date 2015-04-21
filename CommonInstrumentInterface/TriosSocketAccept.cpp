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


#if 0
void
PrintAddrInfo(struct addrinfo *result)
{
    //-----------------------------------------
    // Declare and initialize variables
    INT iRetval;
    int i = 1;
    struct addrinfo *ptr = NULL;
    struct sockaddr_in  *sockaddr_ipv4;
    LPSOCKADDR sockaddr_ip;
    char ipstringbuffer[46];
    DWORD ipbufferlength = 46;
    //-----------------------------------------


       // Retrieve each address and print out the hex bytes
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        printf("getaddrinfo response %d\n", i++);
        printf("\tFlags: 0x%x\n", ptr->ai_flags);
        printf("\tFamily: ");
        switch (ptr->ai_family) {
            case AF_UNSPEC:
                printf("Unspecified\n");
                break;
            case AF_INET:
                printf("AF_INET (IPv4)\n");
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
                printf("\tIPv4 address %s\n",
                    inet_ntoa(sockaddr_ipv4->sin_addr) );
                break;
            case AF_INET6:
                printf("AF_INET6 (IPv6)\n");
                // the InetNtop function is available on Windows Vista and later
                // sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
                // printf("\tIPv6 address %s\n",
                //    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );
                
                // We use WSAAddressToString since it is supported on Windows XP and later
                sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
                // The buffer length is changed by each call to WSAAddresstoString
                // So we need to set it for each iteration through the loop for safety
                ipbufferlength = 46;
                iRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, 
                    ipstringbuffer, &ipbufferlength );
                if (iRetval)
                    printf("WSAAddressToString failed with %u\n", WSAGetLastError() );
                else    
                    printf("\tIPv6 address %s\n", ipstringbuffer);
                break;
            case AF_NETBIOS:
                printf("AF_NETBIOS (NetBIOS)\n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_family);
                break;
        }
        printf("\tSocket type: ");
        switch (ptr->ai_socktype) {
            case 0:
                printf("Unspecified\n");
                break;
            case SOCK_STREAM:
                printf("SOCK_STREAM (stream)\n");
                break;
            case SOCK_DGRAM:
                printf("SOCK_DGRAM (datagram) \n");
                break;
            case SOCK_RAW:
                printf("SOCK_RAW (raw) \n");
                break;
            case SOCK_RDM:
                printf("SOCK_RDM (reliable message datagram)\n");
                break;
            case SOCK_SEQPACKET:
                printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_socktype);
                break;
        }
        printf("\tProtocol: ");
        switch (ptr->ai_protocol) {
            case 0:
                printf("Unspecified\n");
                break;
            case IPPROTO_TCP:
                printf("IPPROTO_TCP (TCP)\n");
                break;
            case IPPROTO_UDP:
                printf("IPPROTO_UDP (UDP) \n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_protocol);
                break;
        }
        printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
        printf("\tCanonical name: %s\n", ptr->ai_canonname);
    }



}
#endif


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
    //  Create a SOCKET for connecting to server
    //
    ListenSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (ListenSocket == -1) {
        LogMessage("socket failed with error: %ld\n", errno);
        freeaddrinfo(res);
        return NULL;
    }

    int yes=1;
    //char yes='1'; // Solaris people use this
    if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        LogMessage("setsockopt failed with error: %ld\n", errno);
        freeaddrinfo(res);
        close(ListenSocket);
        return NULL;
    }


    // Setup the TCP listening socket
    rc = bind( ListenSocket, res->ai_addr, (int)res->ai_addrlen);
    if (rc == -1) {
        LogMessage("bind failed with error: %d\n", errno);
        freeaddrinfo(res);
        close(ListenSocket);
        return NULL;
    }

    freeaddrinfo(res);

    rc = listen(ListenSocket, SOMAXCONN);
    if (rc == -1) {
        LogMessage("listen failed with error: %d\n", errno);
        close(ListenSocket);
        return NULL;
    }

    
    for (;;){

        // 
        //  Accept a client socket - This is the blocking call!
        //
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == -1) {
            LogMessage("accept failed with error: %d\n", errno);
            close(ListenSocket);
            return NULL;
        }

        //
        //  Create a worker thread...
        //
        rc = CreateSocketManager(ClientSocket);
        if (rc != 0){
            close(ListenSocket);
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


