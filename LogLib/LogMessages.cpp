#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "TaWinAssert.h"
#include "LogLib.h"
#include "LogLibPrivate.h"



static LogMsg*
AllocLogLibMsg(void)
{
    LogMsg *Msg = (LogMsg *)malloc(sizeof(LogMsg));
    if(Msg == NULL){
        printf("LogLib(%d): Msg == NULL\n", __LINE__);
        abort();
    }

    Msg->Signature = LOG_LIB_SIGNATURE;

    clock_gettime(CLOCK_REALTIME, &Msg->DateTime);

    Msg->Buffer = NULL;
    Msg->BufferSize = 0;

    return Msg;
}



void __attribute__  ((visibility ("default")))
LogMessage(char *Message, ...)
{
    //----------------------------------------------
    LogMsg *Msg;
    va_list Args;
    //----------------------------------------------

    // 
    //  Check that the library has been initialized
    //
    if(!LogLibInitialized){
        printf("LogLib(%d): !LogLibInitialized\n", __LINE__);
        abort();
    }

    
    Msg = AllocLogLibMsg();

    Msg->Buffer = (char *)malloc(BUFFER_SIZE);
    if(Msg->Buffer == NULL){
        printf("LogLib(%d): Msg->Buffer == NULL\n", __LINE__);
        free(Msg);
        abort();
    }

    va_start(Args, Message);

    Msg->BufferSize = vsnprintf(Msg->Buffer,
                        BUFFER_SIZE,
                        Message,
                        Args);

    if(Msg->BufferSize >= BUFFER_SIZE){
        printf("LogLib(%d): Msg->BufferSize >= BUFFER_SIZE\n", __LINE__);
        free(Msg->Buffer);
        free(Msg);
        abort();
    }

    if(Msg->BufferSize <= 0){
        printf("LogLib(%d): Msg->BufferSize <= 0\n", __LINE__);
        free(Msg->Buffer);
        free(Msg);
        abort();
    }

    QueueLogMsg(Msg);

    va_end(Args);
}



void __attribute__  ((visibility ("default")))
LogMessageBuffer(char *Message, int BufferSize)
{
    //----------------------------------------------
    LogMsg *Msg;
    //----------------------------------------------

    // 
    //  Check that the library has been initialized
    //
    if (!LogLibInitialized){
        printf("LogLib(%d): !LogLibInitialized\n", __LINE__);
        abort();
    }

    Msg = AllocLogLibMsg();

    Msg->Buffer = Message;
    Msg->BufferSize = BufferSize;

    QueueLogMsg(Msg);
}




