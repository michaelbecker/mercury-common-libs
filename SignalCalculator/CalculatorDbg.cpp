/**
 *  @file   calculator_dbg.cpp
 *
 *  @brief  Companion debugging code for calculator.
 *
 *  Subversion Information
 *      - $HeadURL: svn://fortress/NG-Thermal/Embedded/trunk/linux/common/signalcalc/src/calculator_dbg.cpp $
 *      - $Revision: 9678 $
 *      - $Date: 2012-04-26 17:44:38 -0400 (Thu, 26 Apr 2012) $
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
#include <stdarg.h>
#include <time.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <list>

using namespace std;

#include "TaWinAssert.h"
#include "Calculator.h"
#include "CalculatorDbg.h"
#include "CalculatorPrivate.h"
#include "tsort.h"



/////////////////////////////////////////////////////////////////////////////
//
//  Extern data from calculator.cpp that we need access to.  We break some
//  rules of good programming because we need white box access to this 
//  data to debug it.
//
/////////////////////////////////////////////////////////////////////////////
extern list<RawSignal *> RawSignalTable;
extern list<CalcSignal *> CalcSignalTable;
extern double *GetResultRow(int index);
extern int NumberSignals;





void __attribute__  ((visibility ("default")))
SmPrintHeaders(void)
{
    //-------------------------------------
    CalcSignal *cs;
    RawSignal *rs;
    list<RawSignal *>::iterator rsi;
    list<CalcSignal *>::iterator csi;
    //-------------------------------------


    for (rsi = RawSignalTable.begin(); rsi != RawSignalTable.end(); rsi++){
        rs = *rsi;
        printf("%s(%d)   ", rs->DebugName, rs->Id);
    }

    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;
        printf("%s(%d)   ", cs->DebugName, cs->Id);
    }

    printf("\n");
}



void __attribute__  ((visibility ("default")))
SmPrintHeadersToFile(FILE *fp)
{
    //-------------------------------------
    CalcSignal *cs;
    RawSignal *rs;
    list<RawSignal *>::iterator rsi;
    list<CalcSignal *>::iterator csi;
    //-------------------------------------


    for (rsi = RawSignalTable.begin(); rsi != RawSignalTable.end(); rsi++){
        rs = *rsi;
        fprintf(fp, "%s(%d), ", rs->DebugName, rs->Id);
    }

    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        cs = *csi;
        fprintf(fp, "%s(%d), ", cs->DebugName, cs->Id);
    }

    fprintf(fp, "\n");
}
    


void __attribute__  ((visibility ("default")))
SmPrintResults(int index)
{
    double *values = GetResultRow(index);

    ASSERT(values != NULL);

    for (int i = 0; i < NumberSignals; i++){
        if (values[i] > SM_DBL_MAX){
            printf("%11s   ", "-----------");
        }
        else{
            printf("%11.6f   ", values[i]);
        }
    }
    printf("\n");
}



void __attribute__  ((visibility ("default")))
SmPrintFullSignalList(void)
{
    list<RawSignal *>::iterator rsi;
    list<CalcSignal *>::iterator csi;

    printf("Signal List\n");
    printf("-----------------------------------------------------------------------------\n");

    for (rsi = RawSignalTable.begin(); rsi != RawSignalTable.end(); rsi++){
        #ifndef UNDER_CE
        cout << (**rsi) << endl;
        #endif
    }

    for (csi = CalcSignalTable.begin(); csi != CalcSignalTable.end(); csi++){
        #ifndef UNDER_CE
        cout << (**csi) << endl;
        #endif
    }
    printf("-----------------------------------------------------------------------------\n");
}




ostream& operator << (ostream &os, InputSignal &s)
{
    os  << "InputSignal(" << s.Id 
        << ")[" << s.MaxBackwardsIndex 
        << "," << s.MaxForwardsIndex
        << "] reference = " <<  s.Signal
        << "(" << s.Signal->Id
        << ")";

    return os;
}


ostream& operator << (ostream &os, RawSignal &s)
{
    os  << "RawSignal(" << s.Id 
        << ")[" << s.SignalResultsIndex 
        << "]" << endl;

    return os;
}


ostream& operator << (ostream &os, CalcSignal &s)
{
    vector<InputSignal>::iterator isi;
    int i = 0;

    os  << "CalcSignal(" << s.Id 
        << ")[" << s.SignalResultsIndex 
        << "]  numInputSignals = " << s.NumInputSignals
        << "\n\tRelativeMaxBackwardsIndex = " << s.RelativeMaxBackwardsIndex
        << "  AbsoluteMaxBackwardsIndex = " << s.AbsoluteMaxBackwardsIndex
        << "\n\tRelativeMaxForwardsIndex = " << s.RelativeMaxForwardsIndex
        << "  AbsoluteMaxForwardsIndex = " << s.AbsoluteMaxForwardsIndex
        << endl;

    for (isi = s.InputSignalList.begin(); isi != s.InputSignalList.end(); isi++){
        cout    << "\t=> " << (*isi) 
                << "[" << s.InputSignalsResultsIndex[i] 
                << "]" << endl;
        i++;
    }


    return os;
}



static FILE *TraceFp;
static bool Active = false;



static void 
TraceResults(double *Data, int NumberSignals, void *UserData)
{
	(void)UserData;
	
    for (int i = 0; i < NumberSignals; i++){
        if (Data[i] == SM_DBL_DNE){
            fprintf(TraceFp, "%11s, ", "SM_DNE");
        }
        else if (Data[i] == SM_DBL_NAN){
            fprintf(TraceFp, "%11s, ", "SM_NAN");
        }
        else{
            fprintf(TraceFp, "%11.6f, ", Data[i]);
        }
    }

    fprintf(TraceFp, "\n");
}



void __attribute__  ((visibility ("default")))
SmStartSignalTrace(const char *Filename)
{
    if (Active){
        return;
    }
    else{
        Active = true;
    }

    TraceFp = fopen(Filename, "a");

    ASSERT(TraceFp != NULL);

    SmPrintHeadersToFile(TraceFp);
    SmRegisterDataReadyCallback(TraceResults, NULL);
}



void __attribute__  ((visibility ("default")))
SmStopSignalTrace(void)
{
    if (!Active){
        return;
    }
    else{
        Active = false;
    }

    SmUnregisterDataReadyCallback(TraceResults);
    fclose(TraceFp);
}




/**
 *  Set the default debug level here.  This can be 
 *  overridden at the command line.
 */
volatile unsigned int SmDebugLevel = SM_DEFAULT_DEBUG_LEVEL;

//
//  If you are actually calling SmPrintfEx, then the timing may be off anyway...
//
static pthread_mutex_t SignalCalcDebugLock = PTHREAD_MUTEX_INITIALIZER; // ok to static init - mb
static const char *DebugLogFileName = "SmDebug.log";
static int DebugLogFile = -1;


/**
 *
 */
void __attribute__  ((visibility ("default")))
SmPrintfEx(const char *FormatString, ...)
{
    //---------------------------------
    va_list args;
    char Buffer[1024];
    int Offset = 0;
	int BytesWritten;
    struct timespec Time;
    struct tm result;
    //---------------------------------

    clock_gettime(CLOCK_REALTIME, &Time);
    localtime_r(&Time.tv_sec, &result);

	//
	//  Lock Buffer access and file access
	//
	pthread_mutex_lock(&SignalCalcDebugLock);

    //
    //  Trim the day of the week off
    //
    Offset += sprintf(Buffer, "[%02d:%02d:%02d.%03ld]", 
                result.tm_hour, result.tm_min, result.tm_sec, Time.tv_nsec/(1000*1000));

    Offset += sprintf(Buffer + Offset, " ");

    va_start(args, FormatString);

    Offset += vsprintf(Buffer + Offset, FormatString, args);

    va_end(args);
    
    printf("%s", Buffer);

    if (DebugLogFile >= 0){
        BytesWritten = write(DebugLogFile, Buffer, Offset);
        (void)BytesWritten;
    }

    //
    //  Unlock before leaving
    //
	pthread_mutex_unlock(&SignalCalcDebugLock);
}


void __attribute__  ((visibility ("default")))
InitSmPrintf(void)
{
	//----------------------------
    pid_t Pid;
	char Filename[256];
	//----------------------------

	Pid = getpid();

    sprintf(Filename, "%s.%d", DebugLogFileName, Pid);

    DebugLogFile = creat(Filename, 0644);
    ASSERT(DebugLogFile >= 0);
}


void __attribute__  ((visibility ("default")))
SetSmDebugLevel(unsigned int level)
{
    SmDebugLevel = level;
}


