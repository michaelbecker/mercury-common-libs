/**
 *  @file   CalculatorPrivate.h
 *
 *  @brief  Class definitions for the actual raw signal manager.
 *
 *  @note   This should only be included by calculator.cpp!  The public 
 *          API into the calculator is calculator_api.h.
 *
 *  Subversion Information
 *      - $HeadURL: svn://fortress/NG-Thermal/Embedded/trunk/linux/common/signalcalc/src/calculator.h $
 *      - $Revision: 9455 $
 *      - $Date: 2012-04-13 14:20:41 -0400 (Fri, 13 Apr 2012) $
 *      - $Author: mbecker $
 *
 *  Copyright 2011 (c), TA Instruments
 *  
 */
#ifndef CALCULATOR_H__
#define CALCULATOR_H__

#include <string.h>
#include <vector>

#include <iostream>

using namespace std;

#include "Calculator.h"



#define SM_DEFAULT_DEBUG_LEVEL  0x00000000

extern volatile unsigned int SmDebugLevel;


#define SmPrintf(_flag, _format, _args...)	            \
		if ((_flag) & SmDebugLevel){					\
			SmPrintfEx(#_flag " " _format, ## _args);	\
		}


void SmPrintfEx(const char *FormatString, ...);




/**
 *  BaseSignal, an abstract class that RawSignal
 *  and CalcSignal inherit from.
 */
class BaseSignal
{
    public:

        BaseSignal( int Id,
                    const char *debugName,
                    SIGNAL_RESET_FUNCTION reset,
                    void *userData);

        /**
         *  Signal Identifier
         */
        int Id;

        /**
         *  String to assist in debugging.
         */
        const char *DebugName;

        /**
         *  The signal index to get a result of this signal's calculation.
         */
        int SignalResultsIndex;

        /**
         *  Every signal is given an opportunity to know when the calculator 
         *  is resetting everyone.
         */
        SIGNAL_RESET_FUNCTION Reset;
        
        /**
         *  Anything the user wants to save here for their own functions.
         */
        void *UserData;

        /**
         *  If a signal is a Leaf in a dependency tree, then it is a Raw signal.
         *  Otherwise it's not a leaf node and, therefore depends on other 
         *  signals, and by definition is a calculated signal.
         */
        virtual bool IsLeaf() = 0;
};



/**
 *  Signals that do not calculate anything from any other signal.
 *  Typically these should be mapped to raw USB data you want to 
 *  capture.
 */
class  RawSignal : public BaseSignal {

    public:

        RawSignal(  int Id, 
                    const char *debugName,
                    RAW_FUNCTION function,
                    SIGNAL_RESET_FUNCTION reset,
                    void *userData);

        /**
         *  Pointer to the raw difference equation.
         */
        RAW_FUNCTION Function;

        virtual bool IsLeaf() {
            return true;
        }

#ifndef UNDER_CE
    friend ostream& operator << (ostream &os, RawSignal &s);
#endif
};



/**
 *  Helper class for CalcSignal.  This is a class version of 
 *  the INPUT_SIGNAL struct.
 */
class InputSignal {

    public:

        InputSignal(    int Id,
                        int maxBackwardsIndex,
                        int maxForwardsIndex);

        /**
         *  Signal Id
         */
        int Id;

        /**
         *  How far relative backwards we can access.
         */
        int MaxBackwardsIndex;

        /**
         *  How far relative forwards we can access.
         */
        int MaxForwardsIndex;

        /**
         *  Pointer to the real signal we are going to need to get data from.
         */
        BaseSignal *Signal;


    /**
     *  The << can't be a class member, and needs access to the 
     *  internal data in the class.
     */
#ifndef UNDER_CE
    friend ostream& operator << (ostream &os, InputSignal &s);
#endif
};



/**
 *  Signals that are created from other signals.
 */
class  CalcSignal : public BaseSignal {

    public:

        CalcSignal( int Id, 
                    const char *debugName,
                    CALC_FUNCTION function, 
                    SIGNAL_RESET_FUNCTION reset,
                    INPUT_SIGNAL *inputSignalList,
                    void *userData);

        /**
         *  Pointer to the calculated difference equation.
         */
        CALC_FUNCTION Function;

        /**
         *  Get the result from another signal, given an 
         *  absolute index offset.
         */
        double GetInputResult(int index, int inputSignalIndex);

        /**
         *  Function to change the Forwards and Backwards delays 
         *  for a set of input Signals.
         */
        bool ChangeInputSignalIndicies( INPUT_SIGNAL *inputSignalList);

        /**
         *  Input Signal data structures. We save a count, and store the 
         *  data in a vector, and also cache.  The cache is used in the 
         *  GetInputResults() method, to speed up dereferencing the Results
         *  array.
         */
        int                     NumInputSignals;
        vector<InputSignal>     InputSignalList;
        int                     *InputSignalsResultsIndex;  // cache of indicies

        /**
         *  Does NOT take cascading signals into effect.  In fact, for buffering 
         *  you don't NEED to cascade dependent signals to guarantee enough 
         *  buffering.  Max buffering required for a set of Backwards calculated 
         *  signals S is the Max relative backwards index from the set, 
         *  max{S[rel_max_backwards_index]}.
         */
        int RelativeMaxBackwardsIndex;

        /**
         *  Max forward index we directly reference to calculate this signal.  
         */
        int RelativeMaxForwardsIndex;

        /**
         *  The max index forrwards total for all signals that we need to have 
         *  to find this signal.  i.e. We need "some" signal at index x[n+abs...] 
         *  to calculate this at index n.
         */
        int AbsoluteMaxForwardsIndex;

        int AbsoluteMaxBackwardsIndex;

        /**
         *  Where we are right now calculating this signal.  Required for forward 
         *  calculated signals, because where the Signal Manager calculator is 
         *  will NOT be the same as this Index.
         */
        int Index;

        int SignalDneCount;

        /**
         *  Calculated Signals by definition CANNOT be leaf nodes.  These 
         *  must depend on other signals.
         */
        virtual bool IsLeaf() 
        {
            return false;
        }

    /**
     *  The << can't be a class member, and needs access to the 
     *  internal data in the class.
     */
#ifndef UNDER_CE
    friend ostream& operator << (ostream &os, CalcSignal &s);
#endif
};


void 
SendDataReadyCallbacks(double *Data, int NumberSignals);


void 
SendDataSyncedCallbacks(int MinIndexCompleted);


void
FlushDataSyncedCallbacks();


//
//  Need access so we can init these as recursively acquired.
//
extern pthread_mutex_t RunLock;



#endif

