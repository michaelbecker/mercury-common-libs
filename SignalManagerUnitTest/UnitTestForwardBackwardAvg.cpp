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

#define TEST_ITERATIONS 30


static double CorrectResults[TEST_ITERATIONS][6] = 
{
 {    1.000000,   10.100000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    2.000000,   10.200000,  SM_DBL_DNE,   15.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    3.000000,   10.300000,  SM_DBL_DNE,   20.000000,  SM_DBL_DNE,    3.500000,  },
 {    4.000000,   10.400000,    4.000000,   25.000000,   15.000000,    4.500000,  },
 {    5.000000,   10.500000,    5.000000,   30.000000,   20.000000,    5.500000,  },
 {    6.000000,   10.600000,    6.000000,   35.000000,   25.000000,    6.500000,  },
 {    7.000000,   10.700000,    7.000000,   40.000000,   30.000000,    7.500000,  },
 {    8.000000,   10.800000,    8.000000,   45.000000,   35.000000,    8.500000,  },
 {    9.000000,   10.900000,    9.000000,   50.000000,   40.000000,    9.500000,  },
 {   10.000000,   11.000000,   10.000000,   55.000000,   45.000000,   10.500000,  },
 {   11.000000,   11.100000,   11.000000,   60.000000,   50.000000,   11.500000,  },
 {   12.000000,   11.200000,   12.000000,   65.000000,   55.000000,   12.500000,  },
 {   13.000000,   11.300000,   13.000000,   70.000000,   60.000000,   13.500000,  },
 {   14.000000,   11.400000,   14.000000,   75.000000,   65.000000,   14.500000,  },
 {   15.000000,   11.500000,   15.000000,   80.000000,   70.000000,   15.500000,  },
 {   16.000000,   11.600000,   16.000000,   85.000000,   75.000000,   16.500000,  },
 {   17.000000,   11.700000,   17.000000,   90.000000,   80.000000,   17.500000,  },
 {   18.000000,   11.800000,   18.000000,   95.000000,   85.000000,   18.500000,  },
 {   19.000000,   11.900000,   19.000000,  100.000000,   90.000000,   19.500000,  },
 {   20.000000,   12.000000,   20.000000,  105.000000,   95.000000,   20.500000,  },
 {   21.000000,   12.100000,   21.000000,  110.000000,  100.000000,   21.500000,  },
 {   22.000000,   12.200000,   22.000000,  115.000000,  105.000000,   22.500000,  },
 {   23.000000,   12.300000,   23.000000,  120.000000,  110.000000,   23.500000,  },
 {   24.000000,   12.400000,   24.000000,  125.000000,  115.000000,   24.500000,  },
 {   25.000000,   12.500000,   25.000000,  130.000000,  120.000000,   25.500000,  },
 {   26.000000,   12.600000,   26.000000,  135.000000,  125.000000,   26.500000,  },
 {   27.000000,   12.700000,   27.000000,  140.000000,  130.000000,   27.500000,  },
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


static double fcnYUserData = 10.0;

static double  fcnY(void *UserData)
{
    double *y = (double *)UserData;

    *y = *y + 0.1;
    return *y;
}




const int N = 6;

static double  ForwardBackwardWindow(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    int i;
    int start = -(N / 2) + 1;
    int end = (N / 2);
    double sum = 0.0;

    for (i = start; i <= end; i++){
        sum += SmGetInputResult(cs, i, 0);
    }

    sum = sum / N;

    return sum;
}





static double  SimpleAvg3(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double sum = 0.0;

    sum += SmGetInputResult(cs, -3, 0);
    sum += SmGetInputResult(cs, -2, 0);
    sum += SmGetInputResult(cs, -1, 0);
    sum += SmGetInputResult(cs, 0, 0);
    sum += SmGetInputResult(cs, 1, 0);
    sum += SmGetInputResult(cs, 2, 0);
    sum += SmGetInputResult(cs, 3, 0);

    sum = sum / 7.0;

    return sum;
}



static double  Delay1Fwd3Sum(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double sum = 0.0;

    sum += SmGetInputResult(cs, -1, 0);
    sum += SmGetInputResult(cs, 0, 0);
    sum += SmGetInputResult(cs, 1, 0);
    sum += SmGetInputResult(cs, 2, 0);
    sum += SmGetInputResult(cs, 3, 0);

    return sum;
}




static double  Delay3Fwd1Sum(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double sum = 0.0;

    sum += SmGetInputResult(cs, -3, 0);
    sum += SmGetInputResult(cs, -2, 0);
    sum += SmGetInputResult(cs, -1, 0);
    sum += SmGetInputResult(cs, 0, 0);
    sum += SmGetInputResult(cs, 1, 0);

    return sum;
}




enum SignalIds {
    IdInvalid,
    IdFcnX,
    IdFcnY,
    IdSimpleAvg3,
    IdDelay1Fwd3Sum,
    IdDelay3Fwd1Sum,
    IdForwardBackwardWindow,
};


bool UnitTestFwdBkwdWindowAvg(void)
{
    int i;

    printf("Executing UnitTestFwdBkwdWindowAvg\n");

    SmAddRawSignal(IdFcnX, "fcnX", fcnX, NULL, NULL);
    SmAddRawSignal(IdFcnY, "fcnY", fcnY, NULL, &fcnYUserData);


    INPUT_SIGNAL ForwardBackwardWindowInput[] = {
        {IdFcnX, ((N/2) - 1), (N/2)},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL SimpleAvg3Input[] = {
        {IdFcnX, 3, 3},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL Delay1Fwd3SumInput[] = {
        {IdFcnX, 1, 3},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL Delay3Fwd1SumInput[] = {
        {IdFcnX, 3, 1},
        INPUT_SIGNAL_END
    };


    SmAddCalcSignal(IdSimpleAvg3, "SimpleAvg3",  SimpleAvg3,  NULL, SimpleAvg3Input, NULL);
    SmAddCalcSignal(IdDelay1Fwd3Sum, "Delay1Fwd3Sum",  Delay1Fwd3Sum,  NULL, Delay1Fwd3SumInput, NULL);
    SmAddCalcSignal(IdDelay3Fwd1Sum, "Delay3Fwd1Sum",  Delay3Fwd1Sum,  NULL, Delay3Fwd1SumInput, NULL);
    SmAddCalcSignal(IdForwardBackwardWindow, "ForwardBackwardWindow",  ForwardBackwardWindow,  NULL, ForwardBackwardWindowInput, NULL);


    SmInitCalculator();

    if (Verbose)
        SmRegisterDataReadyCallback(PrintResults, NULL);

    SmRegisterDataReadyCallback(VerifyResults, NULL);

    if (Verbose)
        SmPrintHeaders();

    for (i = 0; i<TEST_ITERATIONS; i++){
        SmRunCalculations(NULL);
    }

    return TestPass;
}


