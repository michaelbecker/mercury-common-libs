/**
 *  @file   LogLib.h
 *  @brief  TA Instruments custom Logging library.
 *
 *  This is a multithreaded, queued logging library in which we 
 *  attempt to minimize the time impact writing a log message causes 
 *  the system, or at least the time critical threads.
 */
#ifndef LOG_LIB_H__
#define LOG_LIB_H__


#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Call once at the start of time to initialize the Logging library.
 */
void InitLog(void);

/**
 *  This is the typical log call, works like printf().
 */
void 
LogMessage(char *Message, ...);

/**
 *  This is for pre-formatted message buffers.
 *
 *  When you send Message, you no longer own it.  It will be 
 *  "free"ed by LogLib in the near future once it's written to file.
 */
void 
LogMessageBuffer(char *Message, int BufferSize);


#ifdef __cplusplus
}
#endif



#endif
