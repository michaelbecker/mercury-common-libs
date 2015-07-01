#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <list>


#include "TaWinAssert.h"

using namespace std;


#include "Calculator.h"
#include "CalculatorPrivate.h"


/**
 *  The struct is private to this file.
 */
typedef struct _CALLBACK_DATA{

    DATA_READY_CALLBACK Callback;
    void *UserData;

}CALLBACK_DATA, *PCALLBACK_DATA;


list<PCALLBACK_DATA> DataReadyCallbackList;




bool __attribute__  ((visibility ("default")))
SmRegisterDataReadyCallback(  DATA_READY_CALLBACK Callback,
                            void *UserData)
{
	ASSERT(Callback);

    PCALLBACK_DATA CallbackData = (PCALLBACK_DATA)malloc(sizeof(CALLBACK_DATA));

	ASSERT(CallbackData);

    CallbackData->Callback = Callback;
    CallbackData->UserData = UserData;

    pthread_mutex_lock(&RunLock);

    DataReadyCallbackList.push_front(CallbackData);

    pthread_mutex_unlock(&RunLock);

    return true;
}


bool __attribute__  ((visibility ("default")))
SmUnregisterDataReadyCallback(DATA_READY_CALLBACK Callback)
{
    list<PCALLBACK_DATA>::iterator it;

    pthread_mutex_lock(&RunLock);

    for (   it = DataReadyCallbackList.begin(); 
            it != DataReadyCallbackList.end(); 
            it++){

        PCALLBACK_DATA CallbackData = *it;

        if (CallbackData->Callback == Callback){
            DataReadyCallbackList.remove(CallbackData);
            memset(CallbackData, 0xee, sizeof(DATA_READY_CALLBACK));
            free(CallbackData);
            pthread_mutex_unlock(&RunLock);
            return true;
        }
    }

    pthread_mutex_unlock(&RunLock);

    return false;
}


//
//  You must be holding the RunLock here.
//
void 
SendDataReadyCallbacks(double *Data, int NumberSignals)
{
    list<PCALLBACK_DATA>::iterator it;

    for (   it = DataReadyCallbackList.begin(); 
            it != DataReadyCallbackList.end(); 
            it++){

        PCALLBACK_DATA CallbackData = *it;

        CallbackData->Callback( Data, 
                                NumberSignals, 
                                CallbackData->UserData);
    }
}



