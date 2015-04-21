/**
 *  @file   LogLibPrivate.h
 *  @brief  Shared data between the files in this library.
 */
#ifndef LOGLIB_PRIVATE_H__
#define LOGLIB_PRIVATE_H__

#include <deque>
#include <sys/time.h>

using namespace std;


#define BUFFER_SIZE (512)

#define LOG_LIB_SIGNATURE   'LOGL'

typedef struct LogMsg_ {

    unsigned long Signature;
    char *Buffer;
    int BufferSize;
    struct timespec DateTime;

}LogMsg;


void
QueueLogMsg(LogMsg *Msg);

void
RotateLogFiles(void);

//
//  Volatile init flag.
//
extern volatile bool LogLibInitialized;


#endif
