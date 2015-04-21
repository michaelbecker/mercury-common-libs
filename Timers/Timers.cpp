#include <pthread.h>
#include <semaphore.h>

#include <list>


using namespace std;


#include "TaWinAssert.h"
#include "Timers.h"
#include "Version.h"


/////////////////////////////////////////////////////////////////////////////
//
//  timespec helper functions
//
/////////////////////////////////////////////////////////////////////////////



void __attribute__  ((visibility ("default")))
now_timespec(struct timespec *a)
{
    int rc = clock_gettime(CLOCK_REALTIME, a);
    ASSERT(rc == 0);
}



void __attribute__  ((visibility ("default")))
now_timespec_monotonic_raw(struct timespec *a)
{
    int rc = clock_gettime(CLOCK_MONOTONIC_RAW, a);
    ASSERT(rc == 0);
}



//  c = a + b
void __attribute__  ((visibility ("default")))
add_timespec(struct timespec *c, struct timespec *a, struct timespec *b)
{
    c->tv_sec = a->tv_sec + b->tv_sec;
    c->tv_nsec = a->tv_nsec + b->tv_nsec;
    if (c->tv_nsec >= 1000000000L){
        c->tv_sec++;
        c->tv_nsec -= 1000000000L;
    }
}



//  c = a - b
void __attribute__  ((visibility ("default")))
diff_timespec(struct timespec *c, struct timespec *a, struct timespec *b)
{
    c->tv_sec = a->tv_sec - b->tv_sec;
    c->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (c->tv_nsec < 0){
        c->tv_sec--;
        c->tv_nsec += 1000000000L;
    }
}



void  __attribute__  ((visibility ("default")))
set_timespec_from_ms(struct timespec *a, int ms)
{
    a->tv_sec = ms / 1000;
    a->tv_nsec = (ms % 1000) * 1000 * 1000;
}



//  Expects the timespec's to be normalized.
//  
//  returns:
//      1 =>    a > b
//      0 =>    a = b
//     -1 =>    a < b
int __attribute__  ((visibility ("default")))
compare_timespec(struct timespec *a, struct timespec *b)
{
    if (a->tv_sec > b->tv_sec){
        return 1;
    }
    else if (a->tv_sec < b->tv_sec){
        return -1;
    }
    else if (a->tv_nsec > b->tv_nsec){
        return 1;
    }
    else if (a->tv_nsec < b->tv_nsec){
        return -1;
    }
    else{
        return 0;
    }
}



void __attribute__  ((visibility ("default")))
set_delay_from_now(struct timespec *a, int ms)
{
    struct timespec now;
    struct timespec delta;

    set_timespec_from_ms(&delta, ms);
    now_timespec(&now);

    add_timespec(a, &now, &delta);
}





#define TIMER_CONTROL_SIGNATURE 'TmCt'


typedef struct TimerControl_t_ {

    unsigned int Signature;
    TimerCallback_t Callback;
    void *UserData;
    struct timespec TimeExpired;

}TimerControl_t;


static pthread_mutex_t TimerLock;
static list<TimerControl_t *> ActiveTimers;
static sem_t ExpireTimersSem;

/*
 * http://www.cplusplus.com/reference/list/list/sort/
 *
 * Binary predicate that, taking two values of the same type of those 
 * contained in the list, returns true if the first argument goes 
 * before the second argument in the strict weak ordering it defines, 
 * and false otherwise. This shall be a function pointer or a 
 * function object.
 */
static 
bool SortTimersEarliestFirst(TimerControl_t *a, TimerControl_t *b)
{
    int rc = compare_timespec(&a->TimeExpired, &b->TimeExpired);
    
    //
    //  We want the smaller one first
    //
    if (rc == -1){
        return true;
    }
    else{
        return false;
    }
} 



TIMER_HANDLE __attribute__  ((visibility ("default")))
QueueTimerCallback( TimerCallback_t Callback, 
                    void *UserData,
                    int Delay)
{
    //-----------------------------
    TimerControl_t *Timer;
    //-----------------------------

    Timer = (TimerControl_t *)malloc(sizeof(TimerControl_t));
    ASSERT(Timer);

    Timer->Callback = Callback;
    Timer->UserData = UserData;

    set_delay_from_now(&Timer->TimeExpired, Delay);

    //
    //  Lock ------------------------------------
    //
    pthread_mutex_lock(&TimerLock);

    //
    //  Get it on the list, the Thread will sort it.
    //
    ActiveTimers.push_front(Timer);

    //
    //  Tell the thread the list changed.
    //
    sem_post(&ExpireTimersSem);

    //
    //  Lock ------------------------------------
    //
    pthread_mutex_unlock(&TimerLock);
    
    //
    //  This is how we will cancel this timer.
    //
    return (TIMER_HANDLE)Timer;
}



int __attribute__  ((visibility ("default")))
CancelTimerCallback( TIMER_HANDLE Handle)
{
    //------------------------------------
    TimerControl_t *Timer;
    int rc = 0;
    list<TimerControl_t *>::iterator it;
    //------------------------------------

    Timer = (TimerControl_t *)Handle;

    //
    //  Lock ------------------------------------
    //
    pthread_mutex_lock(&TimerLock);

    //
    //  Look for our timer.
    //
    for (it = ActiveTimers.begin(); it != ActiveTimers.end(); it++){
        
        //
        //  Found it, clean up and leave.  We do not need to 
        //  signal the thread, it's going to wake up anyway.  And 
        //  once it does it makes everything right no matter what.
        //
        if (Timer == *it){
            ActiveTimers.erase(it);
            free(Timer);
            rc = 1;
            break;
        }
    }

    //
    //  Unlock ----------------------------------
    //
    pthread_mutex_unlock(&TimerLock);
    
    return rc;
}


    
static void *
ExpireTimersThread(void *Unused)
{
    //--------------------------------
    (void)Unused;
    struct timespec Timeout;
    struct timespec Now;
    bool TimeoutActive = false;
    int rc;
    TimerControl_t *Timer;
    //--------------------------------

    //
    //  We do not need cleanup.
    //
    pthread_detach(pthread_self());


    for(;;){
        
        if (TimeoutActive){
            sem_timedwait(&ExpireTimersSem, &Timeout);
        }
        else{
            sem_wait(&ExpireTimersSem);
        }


        //
        //  Lock ------------------------------------
        //
        pthread_mutex_lock(&TimerLock);

        if (ActiveTimers.empty()){
            
            TimeoutActive = false;
            
            //
            //  Unlock ----------------------------------
            //
            pthread_mutex_unlock(&TimerLock);
            continue;
        }

        
        //
        //  Re-sort the list, because things may have come and gone 
        //  on it.
        //
        ActiveTimers.sort(SortTimersEarliestFirst);

        //
        //  What time is it now?
        //
        now_timespec(&Now);

        //
        //  Assume we will clean everything up...
        //
        TimeoutActive = false;

        //
        //  Walk the list looking for expired timers
        //
        while ( !ActiveTimers.empty() ){
            
            Timer = ActiveTimers.front();
            
            rc = compare_timespec(&Now, &Timer->TimeExpired);
            
            //
            //  Expired!  Make the callback and clean up/
            //
            if (rc >= 0){
                ActiveTimers.pop_front();
                Timer->Callback(Timer->UserData);
                free(Timer);
            }
            //
            //  Else, there is a timer not expired.
            //
            else {
                //
                //  Use this one as our new timeout delay, 
                //  because we sorted the list.
                //
                Timeout = Timer->TimeExpired;
                TimeoutActive = true;
                break;
            }
        }

        //
        //  Unlock ----------------------------------
        //
        pthread_mutex_unlock(&TimerLock);
    }


    return NULL;
}



void __attribute__  ((visibility ("default")))
InitTimersLib(void)
{
    //----------------------------
    pthread_mutexattr_t attr;
    int rc;
    pthread_t ThreadId;
    //----------------------------

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&TimerLock, &attr);
    ASSERT(rc == 0);
    
    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);


    rc = sem_init(&ExpireTimersSem, 0, 0);
    ASSERT(rc == 0);

    rc = pthread_create(&ThreadId, NULL, ExpireTimersThread, NULL);
    ASSERT(rc == 0);

    LOG_LIBRARY_VERSION();
}


