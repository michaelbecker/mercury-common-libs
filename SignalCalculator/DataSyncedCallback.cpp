#include <stdlib.h>
#include <string.h>


#include <pthread.h>

#include <list>
using namespace std;

#include "TaWinAssert.h"
#include "Calculator.h"
#include "CalculatorDbg.h"
#include "CalculatorPrivate.h"

/**
 *  From calculator.cpp, the signal manager index corresponding to "now".
 */
extern int CurrentGlobalIndex;


/**
 *  Struct is private to this file.
 */
typedef struct _CALLBACK_DATA{

    int IndexTrigger;
    DATA_SYNCED_CALLBACK Callback;
    void *UserData;

}CALLBACK_DATA, *PCALLBACK_DATA;


list<PCALLBACK_DATA> DataSyncedCallbackList;



bool __attribute__  ((visibility ("default")))
SmRegisterDataSyncedCallback( DATA_SYNCED_CALLBACK Callback,
                            void *UserData)
{
	ASSERT(Callback);

    PCALLBACK_DATA CallbackData = (PCALLBACK_DATA)malloc(sizeof(CALLBACK_DATA));

	ASSERT(CallbackData);

    SmPrintf(SmDbgDataSyncedCallbacks, "In SmRegisterDataSyncedCallback()\n");

    CallbackData->Callback      = Callback;
    CallbackData->UserData      = UserData;
    CallbackData->IndexTrigger  = CurrentGlobalIndex;

    //
    //  LOCK ------------------------------------
    //
    pthread_mutex_lock(&RunLock);

    //
    //  MUST queue it at the back!  We will assume order!
    //
    DataSyncedCallbackList.push_back(CallbackData);

    //
    //  UNLOCK ----------------------------------
    //
    pthread_mutex_unlock(&RunLock);

    return true;
}


bool __attribute__  ((visibility ("default")))
SmUnregisterDataSyncedCallback(DATA_SYNCED_CALLBACK Callback)
{
    list<PCALLBACK_DATA>::iterator it;

    //
    //  LOCK ------------------------------------
    //
    pthread_mutex_lock(&RunLock);
            
	SmPrintf(SmDbgDataSyncedCallbacks, "In SmUnregisterDataSyncedCallback()\n");

    for (   it = DataSyncedCallbackList.begin(); 
            it != DataSyncedCallbackList.end(); 
            it++){

        PCALLBACK_DATA CallbackData = *it;

        if (CallbackData->Callback == Callback){
            DataSyncedCallbackList.remove(CallbackData);
            free(CallbackData);

            //
            //  UNLOCK ----------------------------------
            //
            pthread_mutex_unlock(&RunLock);
            return true;
        }
    }

    //
    //  UNLOCK ----------------------------------
    //
    pthread_mutex_unlock(&RunLock);

    return false;
}


/**
 *  Reference:
 *  http://stackoverflow.com/questions/596162/can-you-remove-elements-from-a-stdlist-while-iterating-through-it
 *  
 *  You need to be holding the RunLock here.
 */
void 
SendDataSyncedCallbacks(int MinIndexCompleted)
{
    list<PCALLBACK_DATA>::iterator it;
       
	SmPrintf(SmDbgDataSyncedCallbacks, "In SendDataSyncedCallbacks()\n");

    it = DataSyncedCallbackList.begin(); 
    while (it != DataSyncedCallbackList.end()) {

        PCALLBACK_DATA CallbackData = *it;

        //
        //  If we've passed the trigger point, fire the callback,
        //  and keep looking.
        //
        if ( MinIndexCompleted >= CallbackData->IndexTrigger ){
    
			SmPrintf(SmDbgDataSyncedCallbacks, "Sending a synced callback...\n");

            //
            //  One shot callbacks, take it out of the list, 
            //  and move to the next iterator.
            //
            DataSyncedCallbackList.erase(it++);

            //
            //  Call the function.
            //
            CallbackData->Callback(CallbackData->UserData);

            //
            //  Clean up
            //
            free(CallbackData);
        }
        //
        //  Else since this is an "ordered" list, we can leave.
        //
        else{
            break;
        }
    }
}


/*
 *  You need to be holding the RunLock here.
 */
void 
FlushDataSyncedCallbacks()
{
    list<PCALLBACK_DATA>::iterator it;
    
	SmPrintf(SmDbgDataSyncedCallbacks, "In FlushDataSyncedCallbacks()\n");

    it = DataSyncedCallbackList.begin(); 
    while (it != DataSyncedCallbackList.end()) {
        
		SmPrintf(SmDbgDataSyncedCallbacks, "Flushing a DataSyncedCallback..\n");

        PCALLBACK_DATA CallbackData = *it;

        //
        //  One shot callbacks, take it out of the list, 
        //  and move to the next iterator.
        //
        DataSyncedCallbackList.erase(it++);

        //
        //  Call the function.
        //
        CallbackData->Callback(CallbackData->UserData);

        //
        //  Clean up
        //
        free(CallbackData);
    }
}







