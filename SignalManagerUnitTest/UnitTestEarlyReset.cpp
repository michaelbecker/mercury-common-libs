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
#define TEST_ITERATIONS 4


static double CorrectResults[TEST_ITERATIONS][4] = 
{
 {    1.000000, SM_DBL_DNE, SM_DBL_DNE  },
 {    2.000000, SM_DBL_DNE, SM_DBL_DNE  },
 {    3.000000, SM_DBL_DNE, SM_DBL_DNE  },
 {    4.000000, SM_DBL_DNE, SM_DBL_DNE  },
};

static int CurrentResults = 0;
static bool TestPass = true;

static void 
VerifyResults(double *Data, int NumberSignals, void *UserData)
{
    (void)UserData;

    for (int i = 0; i < NumberSignals; i++){
        if (!DoubleEqual(Data[i], CorrectResults[CurrentResults][i])){
            if (Verbose){
                printf("FAILED VERIFICATION!\n");
            }
            TestPass = false;
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


static double  DelayX(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double delay = SmGetInputResult(cs, -5, 0);

    return delay;
}


static double  FwdDelayX(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double delay = SmGetInputResult(cs, 5, 0);

    return delay;
}



enum SignalIds {
    IdInvalid,
    IdFcnX,
    IdFcnY,
    IdDelayX,
    IdFwdDelayX,
};


bool UnitTestEarlyReset(void)
{
    int i;

    printf("Executing UnitTestEarlyReset\n");

    SmAddRawSignal(IdFcnX, "fcnX", fcnX, NULL, NULL);

    INPUT_SIGNAL DelayXInput[] = {
        {IdFcnX, 5, 0},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL DelayFwdDelayX[] = {
        {IdFcnX, 0, 5},
        INPUT_SIGNAL_END
    };

    SmAddCalcSignal(IdDelayX, "IdDelayX",  DelayX,  NULL, DelayXInput, NULL);
    SmAddCalcSignal(IdFwdDelayX, "IdFwdDelayX",  FwdDelayX,  NULL, DelayFwdDelayX, NULL);


    SmInitCalculator();

    if (Verbose)
        SmRegisterDataReadyCallback(PrintResults, NULL);

    SmRegisterDataReadyCallback(VerifyResults, NULL);

    if (Verbose)
        SmPrintHeaders();

    //
    //  TEST_ITERATIONS should be less than any of the delays.
    //
    for (i = 0; i<TEST_ITERATIONS; i++){
        SmRunCalculations(NULL);
    }
    //
    //  And we need a reset to make sure we can flush all of the data out.
    //
    SmResetCalculator();

    //
    //  Check that all of the rows were sent.
    //
    if (CurrentResults != TEST_ITERATIONS){
        if (Verbose){
            printf("FAILED VERIFICATION!\n");
        }
        TestPass = false;
    }

    return TestPass;


}
