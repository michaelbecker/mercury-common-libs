/**
 *  @file   calculator_api.h
 *
 *  @brief  API for a generic signal / difference eqn calculator.
 *
 *  Subversion Information
 *      - $HeadURL: svn://fortress/NG-Thermal/Embedded/trunk/linux/common/signalcalc/include/calculator_api.h $
 *      - $Revision: 13351 $
 *      - $Date: 2013-08-12 07:42:25 -0400 (Mon, 12 Aug 2013) $
 *      - $Author: kparvataneni $
 *
 *  Copyright 2014 (c), TA Instruments
 *
 *
 *  ===================================================
 *  Callers should be aware of the following 
 *  internal state machine, as to not violate it.
 *  Incorrect calls are handled via the user friendly 
 *  abort() system call.
 *  ===================================================
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
 *  This library is inherently thread safe.
 *  (At least it's supposed to be.)
 *
 */
#ifndef CALCULATOR_API__H__
#define CALCULATOR_API__H__


#ifdef __cplusplus
extern "C" {
#endif



#define SM_FLT_MAX   (1.00e37f)
#define SM_FLT_DNE   (1.01e37f)
#define SM_FLT_NAN   (1.02e37f)
#define SM_FLT_INF   (1.03e37f)

#define SM_DBL_MAX   (1.00e307)
#define SM_DBL_DNE   (1.01e307)
#define SM_DBL_NAN   (1.02e307)
#define SM_DBL_INF   (1.03e307)



/**
 *  Reference handle to a "Signal" that is used to interface 
 *  to various parts of the library.
 */
typedef void * SIGNAL_HANDLE;


/**
 *  This is what a "Raw" signal function must look like.
 */
typedef double ( *RAW_FUNCTION)(void *UserData);


/**
 *  This is what a "Calculated" signal function must look like.
 */
typedef double ( *CALC_FUNCTION)(SIGNAL_HANDLE handle, void *UserData);


/**
 *  This is what a "Reset" callback signal function must look like.
 */
typedef void ( *SIGNAL_RESET_FUNCTION)(void *UserData);


/**
 *  Struct that needs to be passed in to SmAddCalcSignal
 *  as an Array of Input Signals.
 *  The final element must be a sentinal value of 
 *  {0, 0, 0}.  Therefore, 0 can never be a valid signal ID.
 */
typedef struct INPUT_SIGNAL {

    int Id;
    int MaxBackwardsIndex;
    int MaxForwardsIndex;

}*PINPUT_SIGNAL;


/**
 *  Conveience define to signal the end of an INPUT_SIGNAL list.
 */
#define INPUT_SIGNAL_END    {0, 0, 0}


/**
 *  Get the value of an input signal, given the index of the INPUT_SIGNAL 
 *  Array that you passed in from when you called SmAddCalcSignal().
 *
 *  "This should only be called from a CALC_FUNCTION()."
 */
double 
SmGetInputResult(   SIGNAL_HANDLE handle, 
                    int index, 
                    int inputSignalIndex);


/**
 *  "This should only be called from a RAW_FUNCTION() or CALC_FUNCTION()."
 */
void *
SmGetCurrentUserData(void);


/**
 *  This adds a Raw Signal to the Signal Manager.
 */
SIGNAL_HANDLE 
SmAddRawSignal( int Id, 
                const char *debugName,
                RAW_FUNCTION function,
                SIGNAL_RESET_FUNCTION reset,
                void *UserData);


/**
 *  This adds a Raw Signal to the Signal Manager.
 */
SIGNAL_HANDLE 
SmDeprecateSignal(int Id);


/**
 *  This adds a Calculated Signal to the Signal Manager.
 */
SIGNAL_HANDLE 
SmAddCalcSignal(    int Id, 
                    const char *debugName,
                    CALC_FUNCTION function, 
                    SIGNAL_RESET_FUNCTION reset,
                    INPUT_SIGNAL *inputSignalList,
                    void *UserData);


/**
 *  Allows you to change the forward or backward delays on an Input Signal.
 *
 *  ONCE YOU CALL THIS, YOU MUST CALL SmCalculateDelaysAndBufferingWithReset() 
 *  before running the calculator again!
 */
bool 
SmChangeInputSignalIndicies(    SIGNAL_HANDLE handle, 
                                INPUT_SIGNAL *inputSignalList);
bool 
SmChangeInputSignalIndiciesUnsafe(  SIGNAL_HANDLE handle, 
                                    INPUT_SIGNAL *inputSignalList);



/**
 *  To be called EXACTLY ONCE!  Once all Signals have been added.  
 *  You cannot add any signals once this has been called.
 */
void 
SmInitCalculator(void);



/**
 *  Forces a re-initialization of all of the internal buffers, based on the 
 *  requested forward and backwards delays on all of the input signals going 
 *  into the calculated signals.
 *
 *  You do NOT need to call SmResetCalculator() after calling this.
 *  This function will handle it for you.
 */
void 
SmCalculateDelaysAndBufferingWithReset(void);

void 
SmCalculateDelaysAndBufferingWithResetUnsafe(void);


/**
 *  Reset all of the calculations to zero, as if you just 
 *  called SmInitCalculator().
 */
void 
SmResetCalculator(void);


/**
 *  Calling this causes one iteration of the Signal Calculator.
 *  This does NOT mean that all signals have been calculated, 
 *  based on input signal delays.
 */
void 
SmRunCalculations(void *UserData);


/**
 *  You must call this prior to calling SmInitCalculator()
 *  If you pass in a function, then this fcn will be 
 *  called after all calculations are run, and will be 
 *  passed in the UserData you passed in to SmRunCalculations()
 *  or SmRunCalculationsAsync().
 */
typedef void ( *RUN_CLEANUP_FUNCTION)(void *UserData);
void
SmSetCleanupFunction(RUN_CLEANUP_FUNCTION fcn);



/**
 *  Public function to get the latest data value from a Signal.
 */
double 
SmGetRealtimeResult(SIGNAL_HANDLE Handle);



/**
 *  Utility function to get a Real Time value directly from a string.
 *  Slower than the SmGetRealtimeResult() function.  Should not be used 
 *  for anything periodic.
 */
double 
SmGetRealtimeResultFromId(int SignalId);



/**
 *  Utility function to look up a signal handle based on its name.
 *  NULL means it does not exist.
 */
SIGNAL_HANDLE 
SmGetSignalHandle(int SignalId);



/**
 *  Return the offset to be used to get "SignalName" from your 
 *  DATA_READY_CALLBACK().  0 is valid, -1 is an error.
 *
 *  This is an expensive call, and should only be done at the 
 *  start of a test.
 */
int 
SmGetDataReadyDataOffset(int SignalId);


/**
 *  If you want data from signal manager, your callback API needs to look like this.
 */
typedef void ( *DATA_READY_CALLBACK)(double *Data, int NumberSignals, void *UserData);


/**
 *  Register your data callback.
 */
bool 
SmRegisterDataReadyCallback(	DATA_READY_CALLBACK Callback,
								void *UserData);

/**
 *  Unregister your data callback.
 */
bool 
SmUnregisterDataReadyCallback(DATA_READY_CALLBACK Callback);


/**
 *  
 */
typedef void ( *DATA_SYNCED_CALLBACK)(void *UserData);

/**
 *  
 */
bool 
SmRegisterDataSyncedCallback( DATA_SYNCED_CALLBACK Callback,
                            void *UserData);

/**
 *  
 */
bool 
SmUnregisterDataSyncedCallback(DATA_SYNCED_CALLBACK Callback);


/**
 *  If you want the calculations to run async (that is in their own
 *  thread) you need to call this Init function to set it up.
 */
void 
SmInitRunCalculationsAsync(int Priority);


/**
 *  To queue the calculations to run, call this.
 *  This is equivelant to calling SmRunCalculations()
 *  just on a system worker thread.
 */
void 
SmRunCalculationsAsync(void *UserData);


void 
SmAcquireInternalRunLock(void);

void 
SmReleaseInternalRunLock(void);



/**
 *  Special helper defines when a signal needs to behave like a boolean value, 
 *  even though this all works as doubles only.
 *
 *  Return TRUE / FALSE from your signal, but check using IS_TRUE / IS_FALSE.
 */
#define SM_BOOL_SIGNAL_TRUE         2.0
#define SM_BOOL_SIGNAL_FALSE        0.0
#define SM_BOOL_SIGNAL_IS_TRUE(_x)  (_x > 1.0)
#define SM_BOOL_SIGNAL_IS_FALSE(_x) (_x < 1.0)

#define SM_MOD_STATUS_OK                 0
#define SM_MOD_STATUS_WARNING            1
#define SM_MOD_STATUS_ERROR              2



#ifdef __cplusplus
    }
#endif


#endif


