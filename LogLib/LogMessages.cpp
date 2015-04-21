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
    ASSERT(Msg != NULL);

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
    ASSERT(LogLibInitialized == true);

    Msg = AllocLogLibMsg();

    Msg->Buffer = (char *)malloc(BUFFER_SIZE);
    ASSERT(Msg->Buffer != NULL);

    va_start(Args, Message);

    Msg->BufferSize = vsnprintf(Msg->Buffer,
                        BUFFER_SIZE,
                        Message,
                        Args);

    ASSERT(Msg->BufferSize < BUFFER_SIZE);
    ASSERT(Msg->BufferSize > 0);

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
    ASSERT(LogLibInitialized == true);

    Msg = AllocLogLibMsg();

    Msg->Buffer = Message;
    Msg->BufferSize = BufferSize;

    QueueLogMsg(Msg);
}




