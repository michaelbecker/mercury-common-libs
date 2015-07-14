/**
 *  @file   calculator.cpp
 *
 *  @brief  Generic signal / difference eqn calculator
 *
 *  Subversion Information
 *      - $HeadURL: svn://fortress/NG-Thermal/Embedded/trunk/linux/common/signalcalc/src/calculator.cpp $
 *      - $Revision: 12268 $
 *      - $Date: 2012-10-26 16:40:09 -0400 (Fri, 26 Oct 2012) $
 *      - $Author: mbecker $
 *
 *  Copyright 2012 (c), TA Instruments
 *  
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#include <pthread.h>
#include <list>


using namespace std;

#include "Calculator.h"
#include "CalculatorDbg.h"
#include "CalculatorPrivate.h"
#include "tsort.h"
#include "TaWinAssert.h"
#include "Version.h"


#if 0
//
//  These defines convert from the linux API to the 
//  Microsoft VS 2008 API.  Important note, isinf() only 
//  works because we always check for isnan() first in the 
//  code.  !_finite() returns true for nan values too.
//
#define isnan(x_) _isnan(x_)
#define isinf(x_) (!_finite(x_))
#endif


/**
 *  Where all of the calculation results go into.  This is allocated as a
 *  2D array, as follows:
 *
 *  Results[calc index][signal offset]
 *
 *  --------------------
 *  x | 0 1 2 3 4 ... -> (Max Buffering)
 *  y | 0 1 2 3 4 ...
 *  z | 0 1 2 3 4 ...
 *  --------------------
 *  |
 *  V
 *  (Max Signals)
 */
double *Results = NULL;


/**
 *  The index corresponding to "now".
 */
int CurrentGlobalIndex;


/**
 *  The calculated max delayed needed for all signals, given a
 *  set of dependencies.
 */
int MaxBuffering;


/**
 *  Total number of doubles in the Results array.
 */
int TotalCellCount;


/**
 *  The actual list of all Raw Signals added.
 */
list<RawSignal *> RawSignalTable;


/**
 *  The actual list of all Calculated Signals added.
 */
list<CalcSignal *> CalcSignalTable;


/**
 *  Count of items in RawSignalTable.  
 */
int RawNumberSignals = 0;


/**
 *  Count of items in CalcSignalTable.
 */
int CalcNumberSignals = 0;


/**
 *  Raw + Calculated
 */
int NumberSignals;


/**
 *  A list of all the signal names in order.
 */
vector<int>OrderedSignalIdList;


/*
 *  Calculator State Machine for Error Checking
 *  ===========================================
 *
 *                     +-------+
 *                     |       | SmAddRawSignal()
 *          +-------------+    | SmAddCalcSignal()
 *          | SmStateInit | <--+
 *          +-------------+
 *                  ||
 *                  || SmInitCalculator()
 *                  ||
 *                  ||
 *                  ||
 *                  ||   +--------+
 *                  \/   |        | SmRunCalculations()
 *          +----------------+    | SmResetCalculator()
 *  +-----> | SmStateRunable | <--+
 *  |       +----------------+
 *  |               ||
 *  |               ||
 *  |               || 
 *  |               ||
 *  |               ||  SmChangeInputSignalIndicies()
 *  |               ||
 *  |               ||       +--------+
 *  |               \/       |        | SmChangeInputSignalIndicies()
 *  |       +---------------------+   |
 *  +-------| SmStateIndexChanged | <-+
 *  |       +---------------------+
 *  |
 *  |
 *  SmCalculateDelaysAndBufferingWithReset()
 *
 *
 */
enum SmStates{

    SmStateInit,            //  Starting state
    SmStateRunable,         //  We can run here
    SmStateIndexChanged,

}SmState = SmStateInit;



/**
 *  Lock for serializing calls to SmAddRawSignal() and SmAddCalcSignal().
 */
static pthread_mutex_t  AddSignalsLock = PTHREAD_MUTEX_INITIALIZER; // ok to static init - mb



/**
 *  Recursive mutex to sync run time code across multiple threads.
 */
pthread_mutex_t RunLock;


/**
 *  Constructor for InputSignalClass
 */
InputSignal::InputSignal(   int id,
                            int maxBackwardsIndex,
                            int maxForwardsIndex)
    :   Id(id),
		MaxBackwardsIndex(maxBackwardsIndex),
        MaxForwardsIndex(maxForwardsIndex),
        Signal(NULL)
{
}



/**
 *  Constructor for BaseSignal, an abstract class that RawSignal
 *  and CalcSignal inherit from.
 */
BaseSignal::BaseSignal( int id,
                        const char *debugName,
                        SIGNAL_RESET_FUNCTION reset,
                        void *userData)
    :   Id(id),
        DebugName(debugName),
		SignalResultsIndex(-1),
		Reset(reset),
        UserData(userData)
{
}



/**
 *  Internal helper function to get a BaseSignal pointer from a Signal name.
 */
BaseSignal *
GetSignal(int Id)
{
    //-------------------------------------
    CalcSignal *cs;
    RawSignal *rs;
    list<RawSignal *>::iterator rsi;
    list<CalcSignal *>::iterator csi;
    //-------------------------------------


    for (rsi = RawSignalTable.begin(); rsi != RawSignalTable.end(); rsi++){
        rs = *rsi;
		if (rs->Id == Id){
            return rs;
        }
    }

    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;
        if (cs->Id == Id){
            return cs;
        }
    }

    return NULL;
}



/**
 *  Utility function to look up a signal handle based on its name.
 *  NULL means it does not exist.
 */
SIGNAL_HANDLE __attribute__  ((visibility ("default")))
SmGetSignalHandle(int SignalId)
{
    return (GetSignal(SignalId));
}



/**
 *  Constructor for RawSignal, this automatically adds the object 
 *  into the global RawSignalTable.
 */
RawSignal::RawSignal(   int id, 
                        const char *debugName,
                        RAW_FUNCTION function,
                        SIGNAL_RESET_FUNCTION reset,
                        void *userData)
            :   BaseSignal(id, debugName, reset, userData), 
                Function(function)
{
    RawSignalTable.push_back(this);
    RawNumberSignals++;
}



/**
 *  Wrapper for the RawSignal constructor
 */
SIGNAL_HANDLE __attribute__  ((visibility ("default")))
SmAddRawSignal( int Id, 
                const char *debugName,
				RAW_FUNCTION function,
                SIGNAL_RESET_FUNCTION reset,
                void *userData)
{
    pthread_mutex_lock(&AddSignalsLock);

	//	
	//	Cannot Add signal after init!
	//
	ASSERT(SmState == SmStateInit);		
	
    //
    //  Zero is not a valid Signal ID, we use this as a sentinal value.
    //
    ASSERT(Id != 0);

	//	
	//	Duplicate Signal Name
	//
	ASSERT(GetSignal(Id) == NULL);

    RawSignal *rs = new RawSignal(Id, debugName, function, reset, userData);

    pthread_mutex_unlock(&AddSignalsLock);
    
    return rs;
}



static double DeprecatedFcn(void *UserData)
{
    (void)UserData;
    return SM_DBL_DNE;
}



SIGNAL_HANDLE  __attribute__  ((visibility ("default")))
SmDeprecateSignal(int Id)
{
    SIGNAL_HANDLE handle;
    handle = SmAddRawSignal(Id, "Deprecated", DeprecatedFcn, NULL, NULL);
    return handle;
}



/**
 *  Constructor for CalcSignal, this automatically adds the object 
 *  into the global CalcSignalTable.
 */
CalcSignal::CalcSignal( int Id, 
                        const char *debugName,
                        CALC_FUNCTION function,
                        SIGNAL_RESET_FUNCTION reset,
                        INPUT_SIGNAL *inputSignalList,
                        void *userData
                        )
            :   BaseSignal(Id, debugName, reset, userData),
                Function(function),
                RelativeMaxBackwardsIndex(0),
                RelativeMaxForwardsIndex(0),
                AbsoluteMaxForwardsIndex(0),
                AbsoluteMaxBackwardsIndex(0),
                Index(0),
                SignalDneCount(0)
{
    //	
    //	Calculated signals must contain input signals!
    //
    ASSERT(inputSignalList != NULL);

    //
    //  Create InputSignal objects for each INPUT_SIGNAL 
    //  element in the array and add them to our vector.
    //
    NumInputSignals = 0;
    while (inputSignalList->Id != 0){
        InputSignal is(inputSignalList->Id,
                       inputSignalList->MaxBackwardsIndex,
                       inputSignalList->MaxForwardsIndex);
        InputSignalList.push_back(is);
        NumInputSignals++;
        inputSignalList++;
    }
    
    //
    //  Create the cache if we have in
    //
    InputSignalsResultsIndex = (int *)malloc(sizeof(int) * NumInputSignals);
    ASSERT(InputSignalsResultsIndex);

    //
    //  Add us into the CalcSignal list.
    //
    CalcSignalTable.push_back(this);
    CalcNumberSignals++;
}



/**
 *  Wrapper for the CalcSignal constructor
 */
SIGNAL_HANDLE __attribute__  ((visibility ("default")))
SmAddCalcSignal(int Id, 
                const char *debugName,
				CALC_FUNCTION function, 
                SIGNAL_RESET_FUNCTION reset,
                INPUT_SIGNAL *inputSignalList,
                void *userData)
{
    //
    //  LOCK ----------------------------------
    //
    pthread_mutex_lock(&AddSignalsLock);

	//	
	//	Duplicate Signal Name
	//
	ASSERT(GetSignal(Id) == NULL);

    //
    //  Zero is not a valid Signal ID, we use this as a sentinal value.
    //
    ASSERT(Id != 0);

	//	
	//	Cannot Add signal after init!
	//
	ASSERT(SmState == SmStateInit); 

    CalcSignal *cs = new CalcSignal(    Id, 
                                        debugName,
                                        function, 
                                        reset,
                                        inputSignalList,
                                        userData);

    //
    //  UNLOCK ----------------------------------
    //
    pthread_mutex_unlock(&AddSignalsLock);
    
    return cs;
}



/**
 *  Public function to get a data value.
 */
double __attribute__  ((visibility ("default")))
SmGetRealtimeResult(SIGNAL_HANDLE Handle)
{
	//	
	//	Cannot Get RT Signals in this state! Signal manager not Initialized!
	//
	ASSERT(SmState != SmStateInit);	


    pthread_mutex_lock(&RunLock);

    //
    //  If we come in here and the State is not correct, 
    //  return DNE.
    //
    if (SmState == SmStateIndexChanged){
        pthread_mutex_unlock(&RunLock);
        return SM_DBL_DNE;
    }
    
    int index;

    BaseSignal *bs = (BaseSignal *)Handle;

    int SignalIndex = bs->SignalResultsIndex;

    //
    //  If we are a Raw Signal, we use the Global Index.
    //
    if (bs->IsLeaf()){
        index = CurrentGlobalIndex - 1;
    }
    //
    //  Else we need to get into the signal to get it's index.
    //
    else{
        CalcSignal *cs = (CalcSignal*)Handle;
        index = cs->Index - 1;
    }

    if (index < 0){
        pthread_mutex_unlock(&RunLock);
        return SM_DBL_DNE;
    }

    int BufferIndex = index % MaxBuffering;
    double value = Results[SignalIndex + (NumberSignals * BufferIndex)];

    pthread_mutex_unlock(&RunLock);
    
    return value;
}



/**
 *  Utility function to get a Real Time value directly from a string.
 *  Slower than the SmGetRealtimeResult() function.  Should not be used 
 *  for anything periodic.
 */
double __attribute__  ((visibility ("default")))
SmGetRealtimeResultFromId(int SignalId)
{
	//	
	//	Cannot Get RT Signals in this state! Signal manager not runable!
	//
	ASSERT(SmState == SmStateRunable);	

    pthread_mutex_lock(&RunLock);
    
	BaseSignal *bs = GetSignal(SignalId);
    if (bs == NULL){
        pthread_mutex_unlock(&RunLock);
        return SM_DBL_DNE;
    }

    int SignalIndex = bs->SignalResultsIndex;

    int index;

    //
    //  If we are a Raw Signal, we use the Global Index.
    //
    if (bs->IsLeaf()){
        index = CurrentGlobalIndex - 1;
    }
    //
    //  Else we need to get into the signal to get it's index.
    //
    else{
        CalcSignal *cs = (CalcSignal*)bs;
        index = cs->Index - 1;
    }

    if (index < 0){
        pthread_mutex_unlock(&RunLock);    
        return SM_DBL_DNE;
    }

    int BufferIndex = index % MaxBuffering;
    double value = Results[SignalIndex + (NumberSignals * BufferIndex)];

	pthread_mutex_unlock(&RunLock);

    return value;
}



/**
 *  Private helper function to get a ptr to a row in the Results Matrix.
 */
double *
GetResultRow(int index)
{
    if (index < 0){
        return NULL;
    }

    int BufferIndex = index % MaxBuffering;    
    double *value = &Results[NumberSignals * BufferIndex];
    return value;
}



/**
 *  Variable set by SmRunCalculator so that the CalcFunctions 
 *  do not need to receive an index.  Instead they enter the 
 *  relative index into GetInputResult() and that function 
 *  uses this to find the actual index.
 */ 
volatile int VolatileCalcSignalCurrentIndex;



/**
 *  inputSignalIndex is NOT the same as signal index in the global GetResult()
 *  function.  This is the "relative" index as passed in from the 
 *  inputSignalList table.
 */
double 
CalcSignal::GetInputResult(int index, int inputSignalIndex)
{
	//	
	//	CRITICAL ERROR!!! Signal is requesting Signal outside of
	//	the requested inputSignalList!
	//
	ASSERT(inputSignalIndex < NumInputSignals); 

    int RequestedIndex = VolatileCalcSignalCurrentIndex + index;

	//	
	//	CRITICAL ERROR!!! Signal is requesting Index outside of 
	//	it's requested Buffer range!
	//
	ASSERT((RequestedIndex <= (Index + RelativeMaxForwardsIndex)) &&
		(RequestedIndex >= (Index - RelativeMaxBackwardsIndex)));

    //
    //  A signal should never be in the position to request a negative index.
    //
	ASSERT(RequestedIndex >= 0);

    int signal = InputSignalsResultsIndex[inputSignalIndex];
    int BufferIndex = RequestedIndex % MaxBuffering;
    double value = Results[signal + (NumberSignals * BufferIndex)];
    return value;
}



/**
 *  Get the value of an input signal, given the index of the INPUT_SIGNAL 
 *  Array that you passed in from when you called SmAddCalcSignal().
 */
double __attribute__  ((visibility ("default")))
SmGetInputResult(SIGNAL_HANDLE handle, int index, int inputSignalIndex)
{
    CalcSignal *cs = (CalcSignal *)handle;
    return cs->GetInputResult(index, inputSignalIndex);
}




/**
 *  Class method that allows us to change a delay in a signal.
 */
bool CalcSignal::ChangeInputSignalIndicies( INPUT_SIGNAL *inputSignalList)
{
    ASSERT(inputSignalList != NULL);
    int i;

    for (i = 0; i < NumInputSignals; i++){
    
        if (inputSignalList[i].Id == 0){
            break;
        }
        ASSERT(inputSignalList[i].Id == InputSignalList[i].Id);
        InputSignalList[i].MaxBackwardsIndex = inputSignalList[i].MaxBackwardsIndex;
        InputSignalList[i].MaxForwardsIndex = inputSignalList[i].MaxForwardsIndex;
        i++;
    }

    //
    //  We should have seen the sentinal value.
    //
    ASSERT(i == (NumInputSignals + 1));

    return true;
}




/**
 *  Backwards, relative buffering is max of all input backwards buffering
 */
int
FindRelativeBackwardsBuffering(CalcSignal *cs)
{
    //-------------------------------------------
    int runningRelativeMaxBackwardsIndex = 0;
    //-------------------------------------------

    for (int i = 0; i < cs->NumInputSignals; i++){

        if (cs->InputSignalList[i].MaxBackwardsIndex > runningRelativeMaxBackwardsIndex){
            runningRelativeMaxBackwardsIndex = cs->InputSignalList[i].MaxBackwardsIndex;
        }
    }

    return runningRelativeMaxBackwardsIndex;
}



/**
 *  This is not used for buffering, but to skip signals at the begining 
 *  of time, because the input signals may not exist at this time.
 *
 *  We use a recursive search for each signal running down the dependcy chains
 *  using the delays found in InputSignalList until we hit a Leaf node (i.e. a
 *  RawSignal) and we stop.
 */
int
FindAbsoluteBackwardsBuffering(CalcSignal *cs)
{
    //-------------------------------------------
	int *Delays;
    int max = 0;
    CalcSignal *inputCalcSignal;
    //-------------------------------------------

	// 
	//	Dynamic array, each element for each dependent signal
	//	This is what we really want, not possible in MS tools yet.
	//	int Delays[cs->NumInputSignals];    
	//
	Delays = (int *)malloc(sizeof(int) * cs->NumInputSignals);
    ASSERT(Delays != NULL);

    for (int i = 0; i < cs->NumInputSignals; i++){

        Delays[i] = cs->InputSignalList[i].MaxBackwardsIndex;

        if (!cs->InputSignalList[i].Signal->IsLeaf()){
            inputCalcSignal = static_cast<CalcSignal*>(cs->InputSignalList[i].Signal);
            Delays[i] += FindAbsoluteBackwardsBuffering(inputCalcSignal);
        }
    }

    for (int i = 0; i < cs->NumInputSignals; i++){
        if (Delays[i] > max){
            max = Delays[i];
        }
    }

	memset(Delays, 0xAA, sizeof(int) * cs->NumInputSignals);
	free(Delays);

    return (max);
}



/**
 *  Forwards, absolute is sum of all relative buffering for dependency chain
 *  We use a recursive search for each signal running down the dependcy chains
 *  using the delays found in InputSignalList until we hit a Leaf node (i.e. a
 *  RawSignal) and we stop.
 */
int
FindAbsoluteForwardsBuffering(CalcSignal *cs)
{
    //-------------------------------------------
	int *Delays;
	int max = 0;
    CalcSignal *inputCalcSignal;
    //-------------------------------------------

	// 
	//	Dynamic array, each element for each dependent signal
	//	This is what we really want, not possible in MS tools yet.
	//	int Delays[cs->NumInputSignals];    
	//
	Delays = (int *)malloc(sizeof(int)* cs->NumInputSignals);
    ASSERT(Delays != NULL);

    for (int i = 0; i < cs->NumInputSignals; i++){

        Delays[i] = cs->InputSignalList[i].MaxForwardsIndex;

        if (!cs->InputSignalList[i].Signal->IsLeaf()){
            inputCalcSignal = dynamic_cast<CalcSignal*>(cs->InputSignalList[i].Signal);
            Delays[i] += FindAbsoluteForwardsBuffering(inputCalcSignal);
        }
    }

    for (int i = 0; i < cs->NumInputSignals; i++){
        if (Delays[i] > max){
            max = Delays[i];
        }
    }

	memset(Delays, 0xEE, sizeof(int)* cs->NumInputSignals);
	free(Delays);
	
	return (max);
}



/**
 *  Forwards, relative buffering is max of all input forwards buffering
 */
int
FindRelativeForwardsBuffering(CalcSignal *cs)
{
    //-------------------------------------------
    int runningRelativeMaxForwardsIndex = 0;
    //-------------------------------------------

    for (int i = 0; i < cs->NumInputSignals; i++){

        if (cs->InputSignalList[i].MaxForwardsIndex > runningRelativeMaxForwardsIndex){
            runningRelativeMaxForwardsIndex = cs->InputSignalList[i].MaxForwardsIndex;
        }
    }

    return runningRelativeMaxForwardsIndex;
}



/**
 *  Helper function to flush out any and all remaining buffer data so 
 *  it doesn't get thrown away on a Reset.
 */
void 
FlushIncompleteRows(void)
{
    //---------------------------------------------
    CalcSignal *cs;
    list<CalcSignal *>::iterator csi;
    int BufferIndex;
    int CalcSignalIndex = RawNumberSignals;
    int MinIndexCompleted = CurrentGlobalIndex;
    //---------------------------------------------

	SmPrintf(SmDbgCalcLogic, "In FlushIncompleteRows()\n");

    //
    //  If we are at the start of time, just leave.
    //
    if (CurrentGlobalIndex == 0){
        return;
    }

    //
    //  Pass 1 - prep the uncalculated signal spots and figure out how many 
    //  rows we need to flush.
    //
    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){

        cs = *csi;

        //
        //  We will ignore Signals with a negative index still, 
        //  they were all preloaded with DNE at the start of time, 
        //  i.e. GlobalIndex = 0.
        //
        if (cs->Index >= 0){

            //
            //  Figure out far back we need to flush.
            //
            if (cs->Index < MinIndexCompleted){
                MinIndexCompleted = cs->Index;
            }

            //
            //  If we were lagging behind, fill in DNE and catch up.
            //
            if (cs->Index < CurrentGlobalIndex){
                for (int i = cs->Index; i < CurrentGlobalIndex; i++){
                    BufferIndex = i % MaxBuffering;
                    Results[CalcSignalIndex + (BufferIndex * NumberSignals)] = SM_DBL_DNE;
                }
            }
        }
        else{
            //
            //  If we have any uncompleted signals, then _nothing_ has made it
            //  into the data file yet.  We need to flush everything.
            //
            MinIndexCompleted = 0;
            break;
        }

        CalcSignalIndex++;
    }


    //
    //  Pass 2 - send row completes for all the buffered rows to catch up.
    //
    for (int i = MinIndexCompleted; i < CurrentGlobalIndex; i++){

        double *Values = GetResultRow(i);
        SendDataReadyCallbacks(Values, NumberSignals);

        SendDataSyncedCallbacks(i);
    }

}



/**
 *  Reset the calculation engine.
 */
void
PrivateResetCalculator(void)
{
    //-----------------------------------
    int i;
    CalcSignal *cs;
    RawSignal *rs;
    list<RawSignal *>::iterator rsi;
    list<CalcSignal *>::iterator csi;
    //-----------------------------------

	SmPrintf(SmDbgCalcLogic, "In PrivateResetCalculator()\n");

    //
    //  If somehow any are in here, doesn't matter anymore.
    //
    FlushDataSyncedCallbacks();

    //
    //  Clear out the results table, zero is not a good choice!
    //  We will use a special flag float value to designate that 
    //  the signal DNE.
    //
    for (i=0; i<TotalCellCount; i++){
        Results[i] = SM_DBL_DNE;
    }


    for (rsi = RawSignalTable.begin(); rsi != RawSignalTable.end(); rsi++){
        rs = *rsi;
        if (rs->Reset){
            rs->Reset(rs->UserData);
        }
    }

    CurrentGlobalIndex = 0;

    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;
        if (cs->Reset){
            cs->Reset(cs->UserData);
        }
        cs->SignalDneCount = cs->AbsoluteMaxBackwardsIndex;
        cs->Index = CurrentGlobalIndex - cs->AbsoluteMaxForwardsIndex;
    }
}



/**
 *  User API to reset the calculator.
 */
void __attribute__  ((visibility ("default")))
SmResetCalculator(void)
{
	pthread_mutex_lock(&RunLock);
    
	SmPrintf(SmDbgApi, "In SmResetCalculator()\n");

	//	
	//	Cannot Reset Signal Manager in this state!
	//
	ASSERT(SmState == SmStateRunable);
                

    //
    //  We need to flush any remaining data that we are not going to 
    //  get a chance to finish calculating.
    //
    FlushIncompleteRows();

    //
    //  Handle the internals of setting up the signal manager again.
    //
    PrivateResetCalculator();

	pthread_mutex_unlock(&RunLock);
}



/**
 *  Allows us to change a delay in a signal.
 */
bool __attribute__  ((visibility ("default")))
SmChangeInputSignalIndiciesUnsafe(  SIGNAL_HANDLE handle, 
                                    INPUT_SIGNAL *inputSignalList)
{
	SmPrintf(SmDbgApi, "In SmChangeInputSignalIndiciesUnsafe()\n");

    //
    //  If we are transitioning, flush the data, otherwise we could loose it
    //  if the Caller didn't Reset prior.
    //
    if (SmState == SmStateRunable){
        //
        //  We need to flush any remaining data that we are not going to 
        //  get a chance to finish calculating.
        //
        FlushIncompleteRows();

        //
        //  Handle the internals of setting up the signal manager again.
        //
        PrivateResetCalculator();

        //
        //  Remember this is where we are now.
        //
        SmState = SmStateIndexChanged;
    }

	//
	//	Can't change indicies in this state!
	//
	ASSERT(SmState == SmStateIndexChanged);

    CalcSignal *cs = (CalcSignal *)handle;

    bool success = cs->ChangeInputSignalIndicies(inputSignalList);

    return success;
}



/**
 *  Allows us to change a delay in a signal.
 */
bool __attribute__  ((visibility ("default")))
SmChangeInputSignalIndicies(    SIGNAL_HANDLE handle, 
                                INPUT_SIGNAL *inputSignalList)
{
	pthread_mutex_lock(&RunLock);

	SmPrintf(SmDbgApi, "In SmChangeInputSignalIndicies()\n");

    bool success = SmChangeInputSignalIndiciesUnsafe(handle, inputSignalList);

	pthread_mutex_unlock(&RunLock);
    
    return success;
}


/**
 *  Calculate all of the Signal delays and the required buffering.
 */
void 
PrivateCalculateDelaysAndBuffering(void)
{
    //------------------------------------
    CalcSignal *cs;
    list<CalcSignal *>::iterator csi;
	int PriorTotalCellCount;
    //------------------------------------

    MaxBuffering = 0;

    //
    //  Find the relative and absolute backwards buffering required for our
    //  signal list.
    //
    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;

        cs->RelativeMaxBackwardsIndex = FindRelativeBackwardsBuffering(cs);
        cs->RelativeMaxForwardsIndex  = FindRelativeForwardsBuffering(cs);
    }

    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;

        cs->AbsoluteMaxForwardsIndex = FindAbsoluteForwardsBuffering(cs);
        cs->AbsoluteMaxBackwardsIndex = FindAbsoluteBackwardsBuffering(cs);
    }


    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;

        if ((cs->RelativeMaxBackwardsIndex + cs->AbsoluteMaxForwardsIndex) > MaxBuffering){
            MaxBuffering = cs->RelativeMaxBackwardsIndex + cs->AbsoluteMaxForwardsIndex;
        }
    }


    //
    //  One more, for "now".
    //
    MaxBuffering++;

    //
    //  Fill in some final global data.
    //
    NumberSignals = RawNumberSignals + CalcNumberSignals;
	PriorTotalCellCount = TotalCellCount;
    TotalCellCount = NumberSignals * MaxBuffering;

    //
    //  Clean up if we allocated a prior buffer.
    //
    if (Results != NULL){
		memset(Results, 0xAA, PriorTotalCellCount);
        free(Results);
    }

    //
    //  Allocate our results buffer.
    //
    Results = (double *)malloc(sizeof(double) * TotalCellCount);
	ASSERT(Results);
}



/**
 *  Public API
 *
 *  Let the user trigger changing the buffering delays once they are done
 *  changing the delays.
 */
void __attribute__  ((visibility ("default")))
SmCalculateDelaysAndBufferingWithResetUnsafe(void)
{
	SmPrintf(SmDbgApi, "In SmCalculateDelaysAndBufferingWithResetUnsafe()\n");

	//
	//	Attempting to recalculate delays in Signal Manager in an invalid state!
	//
	ASSERT(SmState == SmStateIndexChanged);
                

    SmState = SmStateRunable;

    //
    //  Recalculate the delays
    //
    PrivateCalculateDelaysAndBuffering();

    //
    //  We need to flush any remaining data that we are not going to 
    //  get a chance to finish calculating.
    //
    FlushIncompleteRows();

    //
    //  We MUST reset the calculator here.
    //
    PrivateResetCalculator();    
}



void __attribute__  ((visibility ("default")))
SmCalculateDelaysAndBufferingWithReset(void)
{
	pthread_mutex_lock(&RunLock);

	SmPrintf(SmDbgApi, "In SmCalculateDelaysAndBufferingWithReset()\n");
    SmCalculateDelaysAndBufferingWithResetUnsafe();

	pthread_mutex_unlock(&RunLock);
}



/**
 *  Guarantee that the calculated signals are always called in the 
 *  correct order.
 */
void 
SortSignals(void)
{
    //------------------------------------
    CalcSignal *css;
    CalcSignal *cs;
    RawSignal *rs;
    list<RawSignal *>::iterator rsi;
    list<CalcSignal *>::iterator csi;
    list<CalcSignal *>::iterator csii;
    vector<InputSignal>::iterator isi;

    DirectedAcyclicGraph<BaseSignal *> dag;
    Vertex<BaseSignal *>* v;
    //------------------------------------

    //
    //  We need to build the DAG first so we can sort it.
    //
    for (rsi = RawSignalTable.begin(); rsi != RawSignalTable.end(); rsi++){

        rs = *rsi;
        v = dag.AddVertex(rs);

        for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){

            cs = *csi;

            for (isi = cs->InputSignalList.begin(); isi != cs->InputSignalList.end(); isi++){
                if ((*isi).Signal == rs){
                    v->AddEdge(cs);
                    break;
                }
            }
        }
    }

    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){

        cs = *csi;
        v = dag.AddVertex(cs);

        for (csii = CalcSignalTable.begin(); csii != CalcSignalTable.end(); csii++){

            css = *csii;

            if (css == cs){
                continue;
            }

            for (isi = css->InputSignalList.begin(); isi != css->InputSignalList.end(); isi++){
                if ((*isi).Signal == cs){
                    v->AddEdge(css);
                    break;
                }
            }
        }        
    }

    //
    //  Easy part, sort the DAG
    //
    TopologicalSort<BaseSignal *> ts(dag);

    bool success = ts.Sort();

    //
    //  If it didn't sort, we're done.  Nothing else we can do!
	//	"CRITICAL ERROR!!! Signal dependency graph has cycles!!!!
    //
	ASSERT(success);

    list<BaseSignal *> l = ts.GetSortedList();

    CalcSignalTable.clear();

    for (   list<BaseSignal *>::iterator it = l.begin();
            it != l.end();
            it++){

        BaseSignal *bs = (*it);

        if ( ! bs->IsLeaf() ){
            CalcSignalTable.push_back( static_cast<CalcSignal*>(bs) );
        }
    }
}



/**
 *  Public API
 *  Call this to finish initializing the calculator.
 */
void __attribute__  ((visibility ("default")))
SmInitCalculator(void)
{
    //------------------------------------
    int i = 0;
    CalcSignal *cs;
    RawSignal *rs;
    list<RawSignal *>::iterator rsi;
    list<CalcSignal *>::iterator csi;
    vector<InputSignal>::iterator isi;
    int RawResultIndex = 0;
    int CalcResultIndex = 0;
    pthread_mutexattr_t attr;
    int rc;
    //------------------------------------

	//	
	//	Attempting to re-init Signal Manager!
	//
	ASSERT(SmState == SmStateInit);

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    ASSERT(rc == 0);

#if 0
    //
    //  Double check that both attributes will be set.
    //  Confirmed - these are bit fields in the attr data. - mb 
    //
    int value;

    rc = pthread_mutexattr_getprotocol(&attr, &value);
    ASSERT(rc == 0);
    ASSERT(value == PTHREAD_PRIO_INHERIT);

    rc = pthread_mutexattr_gettype(&attr, &value);
    ASSERT(rc == 0);
    ASSERT(value == PTHREAD_MUTEX_RECURSIVE);
#endif

    rc = pthread_mutex_init(&RunLock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);


    SmState = SmStateRunable;

    //
    //  Fill in the actual Signal pointers first, needed for sort.
    //
    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;
        for (isi = cs->InputSignalList.begin(); isi != cs->InputSignalList.end(); isi++){
            (*isi).Signal = GetSignal((*isi).Id);
        }
    }

    //
    //  Sort the signals, guarantee that they will always be called in the 
    //  correct order!
    //
    SortSignals();

    //
    //  Set up the Signal indicies, so we can get our signals results 
    //  row.
    //
    for (rsi = RawSignalTable.begin(); rsi != RawSignalTable.end(); rsi++){
        rs = *rsi;
        rs->SignalResultsIndex = RawResultIndex;
        RawResultIndex++;
    }
    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;
        cs->SignalResultsIndex = RawResultIndex + CalcResultIndex;
        CalcResultIndex++;
    }
   
    //
    //  Fill in cached result indicies for all the input signals for each input signal.
    //
    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;
        for (i = 0, isi = cs->InputSignalList.begin(); isi != cs->InputSignalList.end(); isi++){
            cs->InputSignalsResultsIndex[i] = (*isi).Signal->SignalResultsIndex;
            i++;
        }
    }

    //
    //  Create a list of all the signal names in order for ease of lookup.
    //
	OrderedSignalIdList.reserve(NumberSignals);

    for (rsi = RawSignalTable.begin(); rsi != RawSignalTable.end(); rsi++){
        rs = *rsi;
		OrderedSignalIdList.push_back(rs->Id);
    }
    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;
		OrderedSignalIdList.push_back(cs->Id);
    }


    //
    //  We are set up, now calculate the buffering and reset the calculator.
    //
    PrivateCalculateDelaysAndBuffering();
    PrivateResetCalculator();    

    LOG_LIBRARY_VERSION();
}



/**
 *  Return the offset to be used to get "SignalName" from your 
 *  DATA_READY_CALLBACK().  0 is valid, -1 is an error.
 *
 *  This is an expensive call, and should only be done at the 
 *  start of a test.
 */
int __attribute__  ((visibility ("default")))
SmGetDataReadyDataOffset(int SignalId)
{
    for (int i = 0; i<NumberSignals; i++){
		if (OrderedSignalIdList[i] == SignalId){
            return i;
        }
    }

    return (-1);
}



static void *CurrentUserData = NULL;

void __attribute__  ((visibility ("default"))) *
SmGetCurrentUserData(void)
{
    return CurrentUserData;
}


static RUN_CLEANUP_FUNCTION CleanupFunction = NULL;

void __attribute__  ((visibility ("default")))
SmSetCleanupFunction(RUN_CLEANUP_FUNCTION fcn)
{
	//	
	//	Cannot set the Cleanup Function after init.
	//
	ASSERT(SmState == SmStateInit);		
	
    CleanupFunction = fcn;
}



/**
 *  The actual engine to be called.
 */
void __attribute__  ((visibility ("default")))
SmRunCalculations(void *UserData)
{
    //-------------------------------------------------
    int RawSignalIndex = 0;
    int CalcSignalIndex = 0;
    int BufferIndex;
    CalcSignal *cs;
    RawSignal *rs;
    list<RawSignal *>::iterator rsi;
    list<CalcSignal *>::iterator csi;
    int MinIndexCompleted;
    bool UncompletedSignals = false;
    double result;
    //-------------------------------------------------

    pthread_mutex_lock(&RunLock);

	SmPrintf(SmDbgApi, "In SmRunCalculations()\n");
    
	//	
	//	Cannot Run Signal Manager in this state!
	//
	ASSERT(SmState == SmStateRunable);
                
    //
    //  Save a snapshot of the UserData for the calculation functions.
    //
    CurrentUserData = UserData;

    //
    //  Raw signals follow the CurrentGlobalIndex
    //
    BufferIndex = CurrentGlobalIndex % MaxBuffering;

    for (rsi = RawSignalTable.begin(); rsi != RawSignalTable.end(); rsi++){
        rs = *rsi;
        result = rs->Function(rs->UserData);

        if (isnan(result)){
            result = SM_DBL_NAN;
        }
        else if (isinf(result)){
            result = SM_DBL_INF;
        }

        Results[RawSignalIndex + (BufferIndex * NumberSignals)] = result;
        RawSignalIndex++;
    }

    //
    //  At minimum we completed all of the raw signals and thus are 
    //  at the CurrentGlobalIndex currently.
    //
    MinIndexCompleted = CurrentGlobalIndex;

    //
    //  CalcSignals start right after the RawSignals.
    //
    CalcSignalIndex = RawSignalIndex;

    //
    //  Calculated Signals are more complex
    //
    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;

        //
        //  If enough time has elapsed, we are actually in the future, so 
        //  we can calculate the signal now at an older index.  Else we flag 
        //  it that we aren't ready for the calculation using 
        //  the UncompletedSignals variable.
        //
        if (cs->Index >= 0){

            //
            //  If the signal depends on data backwards in time, make sure 
            //  enough time has elapsed so the data actually exists.
            //
            if (cs->SignalDneCount){
                cs->SignalDneCount--;
            }
            else{
                //
                //  Each calculated signal has it's own unique index for times when 
                //  it depends on data in the future, so we can delay it until time 
                //  "now" is far enough ahead that the data exists.
                //
                BufferIndex = cs->Index % MaxBuffering;

                //
                //  Save the signal's "now" index 
                //
                VolatileCalcSignalCurrentIndex = cs->Index;

                result = cs->Function(cs, cs->UserData);

                if (isnan(result)){
                    result = SM_DBL_NAN;
                }
                else if (isinf(result)){
                    result = SM_DBL_INF;
                }

                Results[CalcSignalIndex + (BufferIndex * NumberSignals)] = result;
            }

            //
            //  Save which index we calculated.  Signals that DNE count as 
            //  being completed.
            //
            if (cs->Index < MinIndexCompleted){
                MinIndexCompleted = cs->Index;
            }
        }
        else {
            UncompletedSignals = true;
        }

        //
        //  We always move the Signal's Index up.  We preload this at Reset 
        //  backwards from zero to account for forward delays.
        //
        cs->Index++;
        CalcSignalIndex++;
    }


    //
    //  If there's a cleanup function, call it and pass in the 
    //  UserData.  Afterwards zero this out.
    //
    if (CleanupFunction){
        CleanupFunction(CurrentUserData);
        CurrentUserData = NULL;
    }


    //
    //  If we completed everything, we can ship the data off.
    //
    if (!UncompletedSignals){

        //printf("DEBUG Global Index %d - sending index %d\n", CurrentGlobalIndex, MinIndexCompleted);

        SendDataSyncedCallbacks(MinIndexCompleted);

        double *Values = GetResultRow(MinIndexCompleted);
        SendDataReadyCallbacks(Values, NumberSignals);


        //SmPrintResults(MinIndexCompleted);
    }
    else{
        //printf("DEBUG skipping Global Index %d\n", CurrentGlobalIndex);
    }

    CurrentGlobalIndex++;

	pthread_mutex_unlock(&RunLock);
}


