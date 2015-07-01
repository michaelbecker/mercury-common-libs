#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "Calculator.h"
#include "CalculatorDbg.h"


bool DoubleEqual(double a, double b);
void  PrintResults(double *Data, int NumberSignals, void *UserData);
extern bool Verbose;

#define TEST_ITERATIONS 10

static double CorrectResults[TEST_ITERATIONS][2] = 
{
    {   1.000000,     10.100000     },
    {   2.000000,     10.200000     }, 
    {   3.000000,     10.300000     },
    {   4.000000,     10.400000     },
    {   5.000000,     10.500000     },
    {   6.000000,     10.600000     },
    {   7.000000,     10.700000     },
    {   8.000000,     10.800000     },
    {   9.000000,     10.900000     },
    {  10.000000,     11.000000     },
};

static int CurrentResults = 0;
static bool TestPass = true;


static void  
VerifyResults(double *Data, int NumberSignals, void *UserData)
{
    (void)UserData;

    for (int i = 0; i < NumberSignals; i++){
        if (!DoubleEqual(Data[i], CorrectResults[CurrentResults][i])){
            TestPass = false;
            if (Verbose){
                printf("FAILED VERIFICATION!\n");
            }
        }
    }

    CurrentResults++;
}





static double  fcnX(void *UserData)
{
    (void)UserData;

    static double x = 0.0;
    x = x + 1.0;
    return x;
}


static double fcnYUserData = 10.0;

static double  fcnY(void *UserData)
{
    double *y = (double *)UserData;

    *y = *y + 0.1;
    return *y;
}



enum SignalIds {
    IdInvalid,
    IdFcnX,
    IdFcnY,
};


bool UnitTestRawRealTime(void)
{
    int i;

    printf("Executing UnitTestRawRealTime\n");

    SIGNAL_HANDLE HandleX = SmAddRawSignal(IdFcnX, "fcnX", fcnX, NULL, NULL);
    SIGNAL_HANDLE HandleY = SmAddRawSignal(IdFcnY, "fcnY", fcnY, NULL, &fcnYUserData);

    SmInitCalculator();

    if (Verbose)
        SmRegisterDataReadyCallback(PrintResults, NULL);

    SmRegisterDataReadyCallback(VerifyResults, NULL);

    if (Verbose)
        SmPrintHeaders();

    for (i = 0; i<TEST_ITERATIONS; i++){
        SmRunCalculations(NULL);
        double x = SmGetRealtimeResult(HandleX);
        double y = SmGetRealtimeResult(HandleY);
        printf ("RT Signals: %f, %f\n", x, y);
    }

    SmUnregisterDataReadyCallback(VerifyResults);
    if (Verbose)
        SmUnregisterDataReadyCallback(PrintResults);

    return TestPass;
}


