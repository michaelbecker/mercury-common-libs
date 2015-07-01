#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#include <pthread.h>

#include <list>

using namespace std;

#include "WorkQueue.h"

#include "TaWinAssert.h"
#include "Calculator.h"
#include "CalculatorDbg.h"
#include "CalculatorPrivate.h"
#include "TaWinAssert.h"



static WORK_QUEUE_HANDLE WqRunCalculationsAsync = NULL;



void __attribute__  ((visibility ("default")))
SmInitRunCalculationsAsync(int Priority)
{
    ASSERT(WqRunCalculationsAsync == NULL);

    WqRunCalculationsAsync = CreateWorkQueueEx(Priority);

    ASSERT(WqRunCalculationsAsync != NULL);
}



static void 
RunCalculationsCallback(void *UserData)
{
    //
    //  We are in a worker thread here, just call the function.
    //
    SmRunCalculations(UserData);
}



void __attribute__  ((visibility ("default")))
SmRunCalculationsAsync(void *UserData)
{
    ASSERT(WqRunCalculationsAsync != NULL);

    QueueWorkItem(  WqRunCalculationsAsync,
                    RunCalculationsCallback,
                    UserData);
}



void __attribute__  ((visibility ("default")))
SmAcquireInternalRunLock(void)
{
	pthread_mutex_lock(&RunLock);
}



void __attribute__  ((visibility ("default")))
SmReleaseInternalRunLock(void)
{
	pthread_mutex_unlock(&RunLock);
}


