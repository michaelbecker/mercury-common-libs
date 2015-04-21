#include "GpioPrivate.h"
#include <pthread.h>
#include "TaWinAssert.h"
#include "Timers.h"
#include "Version.h"


GpioOutput GpioOutputs[NUM_GPIO_OUTPUTS];
GpioInput GpioInputs[NUM_GPIO_INPUTS];

static pthread_t ThreadId;

static pthread_mutex_t InputLock;
static pthread_mutex_t OutputLock;



#define DEFAULT_INPUT_MONITOR_PERIOD_MS 1000
static int InputMonitorPeriod = DEFAULT_INPUT_MONITOR_PERIOD_MS;


static void *
InputMonitorThread(void *param)
{
    //-------------------------
    (void)param;
    struct timespec Delay;
    //-------------------------

    //
    //  We do not need cleanup.
    //
    pthread_detach(pthread_self());

    set_timespec_from_ms(&Delay, InputMonitorPeriod);
    
    nanosleep(&Delay, NULL);

    for (;;){

        nanosleep(&Delay, NULL);

        //
        //  INPUT LOCK ------------------------------------
        //
        pthread_mutex_lock(&InputLock);
        
        for (int i = 0; i<NUM_GPIO_INPUTS; i++){

            GpioInputs[i].Execute();
        }
    
        //
        //  INPUT UNLOCK ----------------------------------
        //
        pthread_mutex_unlock(&InputLock);
    }

    return NULL;
}



int __attribute__  ((visibility ("default")))
TaGpioRegisterInputChangeCallback(  int GpioInput, 
                                    InputChangeCallback Callback,
                                    void *UserData,
                                    unsigned int Flags)
{
    //------------------------------------
    GpioInputCallbackData *Data;
    int rc;
    //------------------------------------

    ASSERT(GpioInput >= 0);
    ASSERT(GpioInput < NUM_GPIO_INPUTS);
    ASSERT(Callback != NULL);

    Data = new GpioInputCallbackData(Callback, UserData, Flags);
    ASSERT(Data);

    //
    //  INPUT LOCK ------------------------------------
    //
    pthread_mutex_lock(&InputLock);

    if (GpioInputs[GpioInput].IsValid()){

        GpioInputs[GpioInput].AddCallback(Data);
        rc = 0;
    }
    else{

        delete Data;
        rc = -1;
    }

    //
    //  INPUT UNLOCK ----------------------------------
    //
    pthread_mutex_unlock(&InputLock);

    return rc;
}



int __attribute__  ((visibility ("default")))
TaGpioWrite(int GpioOutput, int value)
{
    //----------------
    bool Success;
    //----------------

    ASSERT(GpioOutput >= 0);
    ASSERT(GpioOutput < NUM_GPIO_OUTPUTS);
    
    //
    //  OUTPUT LOCK ------------------------------------
    //
    pthread_mutex_lock(&OutputLock);

    if (GpioOutputs[GpioOutput].IsValid()){

        Success = GpioOutputs[GpioOutput].Write(value);
    }
    else{

        Success = false;
    }

    //
    //  OUTPUT UNLOCK ----------------------------------
    //
    pthread_mutex_unlock(&OutputLock);

    if (Success){
        return 0;
    }
    else{
        return -1;
    }
}



int __attribute__  ((visibility ("default")))
TaGpioInit( char *ConfigFile,       //  Config file for this board.
            int MonitorPeriodInMs   //  The period we want to pool IO for, 0 for default.
            )
{
    //----------------------------
    pthread_mutexattr_t attr;
    int rc;
    //----------------------------

    if (MonitorPeriodInMs <= 100){
        InputMonitorPeriod = DEFAULT_INPUT_MONITOR_PERIOD_MS;
    }
    else if (MonitorPeriodInMs > 10 * 1000){
        InputMonitorPeriod = DEFAULT_INPUT_MONITOR_PERIOD_MS;
    }
    else {
        InputMonitorPeriod = MonitorPeriodInMs;
    }

    if ((rc = ParseConfigFile(ConfigFile)) == -1){
        return rc;
    }

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&InputLock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutex_init(&OutputLock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);
    
    rc = pthread_create(&ThreadId, NULL, InputMonitorThread, NULL);
    ASSERT(rc == 0);

    LOG_LIBRARY_VERSION();

    return rc;
}



