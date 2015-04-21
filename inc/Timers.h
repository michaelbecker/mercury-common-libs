#ifndef TIMERS_H__
#define TIMERS_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <time.h>

void 
InitTimersLib(void);


typedef void (*TimerCallback_t)(void *UserData);
typedef void* TIMER_HANDLE;



TIMER_HANDLE
QueueTimerCallback( TimerCallback_t Callback, 
                    void *UserData,
                    int Delay);     // in ms


//
//  Returns:
//  0 = Timer was not found (may have been called, or is about to)
//  1 = Timer was found and cancelled.
//
int 
CancelTimerCallback(TIMER_HANDLE Handle);





/////////////////////////////////////////////////////////////////////////////
//
//  timespec helper functions
//
/////////////////////////////////////////////////////////////////////////////


// uses CLOCK_REALTIME
void
now_timespec(struct timespec *a);


// uses CLOCK_MONOTONIC_RAW
void
now_timespec_monotonic_raw(struct timespec *a);


//  c = a + b
void
add_timespec(struct timespec *c, struct timespec *a, struct timespec *b);


//  c = a - b
void
diff_timespec(struct timespec *c, struct timespec *a, struct timespec *b);


void 
set_timespec_from_ms(struct timespec *a, int ms);


//  Expects the timespec's to be normalized.
//  
//  returns:
//      1 =>    a > b
//      0 =>    a = b
//     -1 =>    a < b
int 
compare_timespec(struct timespec *a, struct timespec *b);

void
set_delay_from_now(struct timespec *a, int ms);


#ifdef __cplusplus
}
#endif



#endif



