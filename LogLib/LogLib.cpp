#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "CommonFiles.h" // Definition of DATA_ROOT
#include "TaWinAssert.h"
#include "LogLib.h"
#include "LogLibPrivate.h"
#include "Version.h"


#define MERCURY_CORTEX_LOG_FILE_0 (DATA_ROOT "MercuryLog.0.txt")
#define MERCURY_CORTEX_LOG_FILE_1 (DATA_ROOT "MercuryLog.1.txt")
#define MERCURY_CORTEX_LOG_FILE_2 (DATA_ROOT "MercuryLog.2.txt")
#define MERCURY_CORTEX_LOG_FILE_3 (DATA_ROOT "MercuryLog.3.txt")
#define MERCURY_CORTEX_LOG_FILE_4 (DATA_ROOT "MercuryLog.4.txt")
#define MERCURY_CORTEX_LOG_FILE_5 (DATA_ROOT "MercuryLog.5.txt")


#define MAX_LOG_FILE_SIZE ( 1 * (1024 * 1024))


static int LogFile = -1;

static pthread_mutex_t LogMsgQueueLock;
static pthread_t ThreadId;
static sem_t hLogMsgEvent;
static deque<LogMsg *> LogMsgQueue;


volatile bool LogLibInitialized = false;


void
QueueLogMsg(LogMsg *Msg)
{
    if(Msg->Signature != LOG_LIB_SIGNATURE){
        printf("LogLib(%d): Msg->Signature != LOG_LIB_SIGNATURE\n", __LINE__);
        abort();
    }
    

    //
    //  LOCK --------------------------------------------------------
    //
    pthread_mutex_lock(&LogMsgQueueLock);

    LogMsgQueue.push_front(Msg);
    sem_post(&hLogMsgEvent);

    //
    //  UNLOCK ------------------------------------------------------
    //
    pthread_mutex_unlock(&LogMsgQueueLock);
}



bool
DequeueLogMsg(LogMsg **Msg)
{
    bool MsgFound;

    //
    //  LOCK --------------------------------------------------------
    //
    pthread_mutex_lock(&LogMsgQueueLock);

    if (!LogMsgQueue.empty()){
        MsgFound = true;
        *Msg = LogMsgQueue.back();
        LogMsgQueue.pop_back();

        if((*Msg)->Signature != LOG_LIB_SIGNATURE){
            printf("LogLib(%d): (*Msg)->Signature != LOG_LIB_SIGNATURE\n", __LINE__);
            abort();
        }
    }
    else{
        MsgFound = false;
    }

    //
    //  UNLOCK ------------------------------------------------------
    //
    pthread_mutex_unlock(&LogMsgQueueLock);

    return MsgFound;
}



void
RotateLogFiles(void)
{
    //
    //  LOCK --------------------------------------------------------
    //
    pthread_mutex_lock(&LogMsgQueueLock);

    if (LogFile != -1){
        close(LogFile);
    }

    (void)unlink(MERCURY_CORTEX_LOG_FILE_5);
    (void)rename(MERCURY_CORTEX_LOG_FILE_4, MERCURY_CORTEX_LOG_FILE_5);
    (void)rename(MERCURY_CORTEX_LOG_FILE_3, MERCURY_CORTEX_LOG_FILE_4);
    (void)rename(MERCURY_CORTEX_LOG_FILE_2, MERCURY_CORTEX_LOG_FILE_3);
    (void)rename(MERCURY_CORTEX_LOG_FILE_1, MERCURY_CORTEX_LOG_FILE_2);
    (void)rename(MERCURY_CORTEX_LOG_FILE_0, MERCURY_CORTEX_LOG_FILE_1);

    LogFile = creat(MERCURY_CORTEX_LOG_FILE_0, 0644);

    //
    //  UNLOCK ------------------------------------------------------
    //
    pthread_mutex_unlock(&LogMsgQueueLock);
}



static void
CheckLogRotation(void)
{
    struct stat st;

    int rc = fstat(LogFile, &st);

    if (rc != 0){
        return;
    }

    if (st.st_size > MAX_LOG_FILE_SIZE){
        RotateLogFiles();
    }
}


const char *MonthStr[] = 
{
    "Jan",    "Feb",    "Mar",
    "Apr",    "May",    "Jun",    "Jul",
    "Aug",    "Sep",    "Oct",    "Nov",
    "Dec",    "---"
};

const char*Wkday[]  = 
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};


static void
WriteLogMsgToFile(LogMsg *Msg)
{
    //----------------------------------------------
    char ScratchBuffer[80];
    int Offset;
    ssize_t BytesWritten;
    struct tm result;
    //----------------------------------------------

    if(Msg->Signature != LOG_LIB_SIGNATURE){
        printf("LogLib(%d): Msg->Signature != LOG_LIB_SIGNATURE\n", __LINE__);
        abort();
    }

    localtime_r(&(Msg->DateTime.tv_sec), &result);

    Offset = snprintf( ScratchBuffer, 
                        80, 
                        "%s %d, %d %2d:%02d:%02d"
                        " [%09ld] ", 

                        MonthStr[result.tm_mon],
                        result.tm_mday,
                        result.tm_year + 1900,  // Otherwise the year is 114.
                        result.tm_hour,
                        result.tm_min,
                        result.tm_sec,
                        Msg->DateTime.tv_nsec
                        );

    BytesWritten = write(LogFile, ScratchBuffer, Offset);
    if(BytesWritten != Offset){
        printf("LogLib(%d): BytesWritten != Offset\n", __LINE__);
        abort();
    }

    BytesWritten = write(LogFile, Msg->Buffer, Msg->BufferSize);
    if(BytesWritten != Msg->BufferSize){
        printf("LogLib(%d): BytesWritten != Msg->BufferSize\n", __LINE__);
        abort();
    }

    BytesWritten = write(LogFile, "\n", 1);
    if(BytesWritten != 1){
        printf("LogLib(%d): BytesWritten != 1\n", __LINE__);
        abort();
    }

	memset(Msg->Buffer, 0x11, Msg->BufferSize);
    free(Msg->Buffer);

	memset(Msg, 0x11, sizeof(LogMsg));
    free(Msg);
}



static void * 
LogLibThread(void * lpParam )
{
    (void)lpParam;
    //----------------------------------------------
    LogMsg *Msg;
    bool MsgFound = true;
    //----------------------------------------------

    //
    //  We do not need cleanup.
    //
    pthread_detach(pthread_self());

    for(;;){

        sem_wait(&hLogMsgEvent);

        do {
            MsgFound = DequeueLogMsg(&Msg);

            if (MsgFound){

                if(Msg->Signature != LOG_LIB_SIGNATURE){
                    printf("LogLib(%d): Msg->Signature != LOG_LIB_SIGNATURE\n", __LINE__);
                    abort();
                }

                WriteLogMsgToFile(Msg);
            }

        } while(MsgFound);

        CheckLogRotation();
    }
}



void __attribute__  ((visibility ("default")))
InitLog(void) 
{
    //----------------------------
    pthread_mutexattr_t attr;
    int rc;
    //----------------------------

    rc = pthread_mutexattr_init(&attr);
    if(rc != 0){
        printf("LogLib(%d): rc != 0\n", __LINE__);
        abort();
    }

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    if(rc != 0){
        printf("LogLib(%d): rc != 0\n", __LINE__);
        abort();
    }
    
    rc = pthread_mutex_init(&LogMsgQueueLock, &attr);
    if(rc != 0){
        printf("LogLib(%d): rc != 0\n", __LINE__);
        abort();
    }
    
    rc = pthread_mutexattr_destroy(&attr);
    if(rc != 0){
        printf("LogLib(%d): rc != 0\n", __LINE__);
        abort();
    }
    

    RotateLogFiles();
    
    LogFile = open(MERCURY_CORTEX_LOG_FILE_0, O_RDWR | O_APPEND | O_CREAT, 0644);
    if(LogFile == -1){
        printf("LogLib(%d): LogFile == -1\n", __LINE__);
        abort();
    }

    rc = sem_init(&hLogMsgEvent, 0, 0);
    if(rc != 0){
        printf("LogLib(%d): rc != 0\n", __LINE__);
        abort();
    }

    rc = pthread_create(&ThreadId, NULL, LogLibThread, NULL);
    if(rc != 0){
        printf("LogLib(%d): rc != 0\n", __LINE__);
        abort();
    }

    // 
    //  Flag that Library is initialized.
    //
    LogLibInitialized = true;

    LOG_LIBRARY_VERSION();
}


