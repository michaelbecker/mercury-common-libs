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


static double CorrectResults[TEST_ITERATIONS][12] = 
{
 {    1.000000,   10.100000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    2.000000,   10.200000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    3.000000,   10.300000,    2.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    4.000000,   10.400000,    3.000000,  SM_DBL_DNE,  SM_DBL_DNE,   10.250000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    5.000000,   10.500000,    4.000000,  SM_DBL_DNE,    1.000000,   10.350000,  SM_DBL_DNE,    6.650000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    6.000000,   10.600000,    5.000000,    3.500000,    2.000000,   10.450000,  SM_DBL_DNE,    7.200000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    7.000000,   10.700000,    6.000000,    4.500000,    3.000000,   10.550000,   10.400000,    7.750000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    8.000000,   10.800000,    7.000000,    5.500000,    4.000000,   10.650000,   10.500000,    8.300000,   21.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    9.000000,   10.900000,    8.000000,    6.500000,    5.000000,   10.750000,   10.600000,    8.850000,   25.000000,   73.500000,  SM_DBL_DNE,  },
 {   10.000000,   11.000000,    9.000000,    7.500000,    6.000000,   10.850000,   10.700000,    9.400000,   29.000000,   74.200000,  SM_DBL_DNE,  },
 {   11.000000,   11.100000,   10.000000,    8.500000,    7.000000,   10.950000,   10.800000,    9.950000,   33.000000,   74.900000,  SM_DBL_DNE,  },
 {   12.000000,   11.200000,   11.000000,    9.500000,    8.000000,   11.050000,   10.900000,   10.500000,   37.000000,   75.600000,  422.200000,  },
 {   13.000000,   11.300000,   12.000000,   10.500000,    9.000000,   11.150000,   11.000000,   11.050000,   41.000000,   76.300000,  441.000000,  },
 {   14.000000,   11.400000,   13.000000,   11.500000,   10.000000,   11.250000,   11.100000,   11.600000,   45.000000,   77.000000,  459.800000,  },
 {   15.000000,   11.500000,   14.000000,   12.500000,   11.000000,   11.350000,   11.200000,   12.150000,   49.000000,   77.700000,  478.600000,  },
 {   16.000000,   11.600000,   15.000000,   13.500000,   12.000000,   11.450000,   11.300000,   12.700000,   53.000000,   78.400000,  497.400000,  },
 {   17.000000,   11.700000,   16.000000,   14.500000,   13.000000,   11.550000,   11.400000,   13.250000,   57.000000,   79.100000,  516.200000,  },
 {   18.000000,   11.800000,   17.000000,   15.500000,   14.000000,   11.650000,   11.500000,   13.800000,   61.000000,   79.800000,  535.000000,  },
 {   19.000000,   11.900000,   18.000000,   16.500000,   15.000000,   11.750000,   11.600000,   14.350000,   65.000000,   80.500000,  553.800000,  },
 {   20.000000,   12.000000,   19.000000,   17.500000,   16.000000,   11.850000,   11.700000,   14.900000,   69.000000,   81.200000,  572.600000,  },
 {   21.000000,   12.100000,   20.000000,   18.500000,   17.000000,   11.950000,   11.800000,   15.450000,   73.000000,   81.900000,  591.400000,  },
 {   22.000000,   12.200000,   21.000000,   19.500000,   18.000000,   12.050000,   11.900000,   16.000000,   77.000000,   82.600000,  610.200000,  },
 {   23.000000,   12.300000,   22.000000,   20.500000,   19.000000,   12.150000,   12.000000,   16.550000,   81.000000,   83.300000,  629.000000,  },
 {   24.000000,   12.400000,   23.000000,   21.500000,   20.000000,   12.250000,   12.100000,   17.100000,   85.000000,   84.000000,  647.800000,  },
 {   25.000000,   12.500000,   24.000000,   22.500000,   21.000000,   12.350000,   12.200000,   17.650000,   89.000000,   84.700000,  666.600000,  },
 {   26.000000,   12.600000,   25.000000,   23.500000,   22.000000,   12.450000,   12.300000,   18.200000,   93.000000,   85.400000,  685.400000,  },
 {   27.000000,   12.700000,   26.000000,   24.500000,   23.000000,   12.550000,   12.400000,   18.750000,   97.000000,   86.100000,  704.200000,  },
 {   28.000000,   12.800000,   27.000000,   25.500000,   24.000000,   12.650000,   12.500000,   19.300000,  101.000000,   86.800000,  723.000000,  },
 {   29.000000,   12.900000,   28.000000,   26.500000,   25.000000,   12.750000,   12.600000,   19.850000,  105.000000,   87.500000,  741.800000,  },
 {   30.000000,   13.000000,   29.000000,   27.500000,   26.000000,   12.850000,   12.700000,   20.400000,  109.000000,   88.200000,  760.600000,  },

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


static double  BkwdAvg3X(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double x_n = SmGetInputResult(cs, 0, 0);
    double x_n1 = SmGetInputResult(cs, -1, 0);
    double x_n2 = SmGetInputResult(cs, -2, 0);

    double avg = (x_n2 + x_n1 + x_n) / 3.0;
    return avg;
}



static double  BkwdAvg6X(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double x_n = SmGetInputResult(cs, 0, 0);
    double x_n1 = SmGetInputResult(cs, -1, 0);
    double x_n2 = SmGetInputResult(cs, -2, 0);
    double x_n3 = SmGetInputResult(cs, -3, 0);
    double x_n4 = SmGetInputResult(cs, -4, 0);
    double x_n5 = SmGetInputResult(cs, -5, 0);

    double avg = (x_n5 + x_n4 + x_n3 + x_n2 + x_n1 + x_n) / 6.0;
    return avg;
}



static double  BkwdAvg4Y(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double x_n = SmGetInputResult(cs, 0, 0);
    double x_n1 = SmGetInputResult(cs, -1, 0);
    double x_n2 = SmGetInputResult(cs, -2, 0);
    double x_n3 = SmGetInputResult(cs, -3, 0);

    double avg = (x_n3 + x_n2 + x_n1 + x_n) / 4.0;
    return avg;
}



static double  BkwdAvg7Y(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double x_n = SmGetInputResult(cs, 0, 0);
    double x_n1 = SmGetInputResult(cs, -1, 0);
    double x_n2 = SmGetInputResult(cs, -2, 0);
    double x_n3 = SmGetInputResult(cs, -3, 0);
    double x_n4 = SmGetInputResult(cs, -4, 0);
    double x_n5 = SmGetInputResult(cs, -5, 0);
    double x_n6 = SmGetInputResult(cs, -6, 0);

    double avg = (x_n6 + x_n5 + x_n4 + x_n3 + x_n2 + x_n1 + x_n) / 7.0;
    return avg;
}




static double  BkwdAvg5XY(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double x_n = SmGetInputResult(cs, 0, 0);
    double x_n1 = SmGetInputResult(cs, -1, 0);
    double x_n2 = SmGetInputResult(cs, -2, 0);
    double x_n3 = SmGetInputResult(cs, -3, 0);
    double x_n4 = SmGetInputResult(cs, -4, 0);

    double y_n = SmGetInputResult(cs, 0, 1);
    double y_n1 = SmGetInputResult(cs, -1, 1);
    double y_n2 = SmGetInputResult(cs, -2, 1);
    double y_n3 = SmGetInputResult(cs, -3, 1);
    double y_n4 = SmGetInputResult(cs, -4, 1);

    double avg = ((x_n4 + x_n3 + x_n2 + x_n1 + x_n) + (y_n4 + y_n3 + y_n2 + y_n1 + y_n)) / 10.0;
    return avg;
}


static double  Delay5X(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double x_n = SmGetInputResult(cs, -4, 0);

    double avg = x_n;
    return avg;
}



static double  FcnX9A(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double x1 = SmGetInputResult(cs, 0, 0);
    double x2 = SmGetInputResult(cs, -1, 0);

    double x3 = SmGetInputResult(cs, -1, 1);
    double x4 = SmGetInputResult(cs, -2, 1);

    double sum = x1 + x2 + x3 + x4;
    return sum;
}



static double  FcnY11A(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double a = SmGetInputResult(cs, -1, 0);
    double b = SmGetInputResult(cs, -2, 0);
    double c = SmGetInputResult(cs, -3, 0);
    double d = SmGetInputResult(cs, -4, 0);

    double e = SmGetInputResult(cs, 0, 1);
    double f = SmGetInputResult(cs, -1, 1);
    double g = SmGetInputResult(cs, -2, 1);

    double sum = a + b + c + d + e + f + g;
    return sum;
}



static double  FcnXYA(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double a = SmGetInputResult(cs, 0, 0);
    double b = SmGetInputResult(cs, -1, 0);
    double c = SmGetInputResult(cs, -2, 0);
    double d = SmGetInputResult(cs, -3, 0);

    double e = SmGetInputResult(cs, 0, 1);
    double f = SmGetInputResult(cs, -1, 1);
    double g = SmGetInputResult(cs, -2, 1);
    double h = SmGetInputResult(cs, -3, 1);

    double sum = a + b + c + d + e + f + g + h;
    return sum;
}



enum SignalIds {
    IdInvalid,
    IdFcnX,
    IdFcnY,
    IdBkwdAvg3X,
    IdBkwdAvg6X,
    IdBkwdAvg4Y,
    IdBkwdAvg7Y,
    IdBkwdAvg5XY,
    IdDelay5X,
    IdFcnX9A,
    IdFcnY11A,
    IdFcnXYA,
};


bool UnitTestBkwd(void)
{
    int i;

    printf("Executing UnitTestBkwd\n");

    SmAddRawSignal(IdFcnX, "fcnX", fcnX, NULL, NULL);
    SmAddRawSignal(IdFcnY, "fcnY", fcnY, NULL, &fcnYUserData);

    INPUT_SIGNAL BkwdAvg3XInput[] = {
        {IdFcnX, 2, 0},
        INPUT_SIGNAL_END
    };
    SmAddCalcSignal(IdBkwdAvg3X, "BkwdAvg3X", BkwdAvg3X, NULL, BkwdAvg3XInput, NULL);


    INPUT_SIGNAL BkwdAvg6XInput[] = {
        {IdFcnX, 5, 0},
        INPUT_SIGNAL_END
    };
    SmAddCalcSignal(IdBkwdAvg6X, "BkwdAvg6X", BkwdAvg6X, NULL, BkwdAvg6XInput, NULL);


    INPUT_SIGNAL BkwdAvg4YInput[] = {
        {IdFcnY, 3, 0},
        INPUT_SIGNAL_END
    };
    SmAddCalcSignal(IdBkwdAvg4Y, "BkwdAvg4Y", BkwdAvg4Y, NULL, BkwdAvg4YInput, NULL);


    INPUT_SIGNAL BkwdAvg7YInput[] = {
        {IdFcnY, 6, 0},
        INPUT_SIGNAL_END
    };
    SmAddCalcSignal(IdBkwdAvg7Y, "BkwdAvg7Y", BkwdAvg7Y, NULL, BkwdAvg7YInput, NULL);


    INPUT_SIGNAL BkwdAvg5XYInput[] = {
        {IdFcnX, 4, 0},
        {IdFcnY, 4, 0},
        INPUT_SIGNAL_END
    };
    SmAddCalcSignal(IdBkwdAvg5XY, "BkwdAvg5XY", BkwdAvg5XY, NULL, BkwdAvg5XYInput, NULL);


    INPUT_SIGNAL Delay5XInput[] = {
        {IdFcnX, 4, 0},
        INPUT_SIGNAL_END
    };
    SmAddCalcSignal(IdDelay5X, "Delay5X", Delay5X, NULL, Delay5XInput, NULL);


    INPUT_SIGNAL FcnX9AInput[] = {
        {IdBkwdAvg3X, 1, 0},
        {IdBkwdAvg6X, 2, 0},
        INPUT_SIGNAL_END
    };
    SmAddCalcSignal(IdFcnX9A, "FcnX9A", FcnX9A, NULL, FcnX9AInput, NULL);


    INPUT_SIGNAL FcnY11AInput[] = {
        {IdBkwdAvg4Y, 4, 0},
        {IdBkwdAvg7Y, 2, 0},
        INPUT_SIGNAL_END
    };
    SmAddCalcSignal(IdFcnY11A, "FcnY11A", FcnY11A, NULL, FcnY11AInput, NULL);


    INPUT_SIGNAL FcnXYAInput[] = {
        {IdFcnX9A, 3, 0},
        {IdFcnY11A, 3, 0},
        INPUT_SIGNAL_END
    };
    SmAddCalcSignal(IdFcnXYA, "FcnXYA", FcnXYA, NULL, FcnXYAInput, NULL);


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


