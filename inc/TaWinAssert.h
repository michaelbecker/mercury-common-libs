/**
 *  @file   TaWinAssert.h
 *  @brief  Custom ASSERT code for the Mercury project.
 *
 *  This ASSERT macro tries to guarantee that the message makes it 
 *  into the log file before issuing an abort() call.
 */
#ifndef TA_WIN_ASSERT_H__
#define TA_WIN_ASSERT_H__

#include <stdlib.h>
#include <unistd.h>
#include "LogLib.h"

#ifdef __cplusplus
   extern "C" {
#endif


#define ASSERT(_TestCondition)                                  \
       if (!(_TestCondition)){                                  \
           LogMessage("ASSERT FAILED! \"%s\" %s:%d\n",          \
                        #_TestCondition, __FILE__, __LINE__);   \
           sleep(5);                                            \
           abort();                                             \
       }


#ifdef __cplusplus
    }
#endif

#endif

