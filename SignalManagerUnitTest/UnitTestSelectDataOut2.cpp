#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "Calculator.h"
#include "CalculatorDbg.h"


#define SM_VERIFY 1


bool DoubleEqual(double a, double b);
void  PrintResults(double *Data, int NumberSignals, void *UserData);
extern bool Verbose;

#define TEST_ITERATIONS 30

static bool TestPass = true;

static int xOffset;
static int xCalcOffset;
static int zOffset;
static int zCalcOffset;


#if SM_VERIFY
static double CorrectResults[TEST_ITERATIONS][12] = 
{
//     fcnX          fcnZ         Calc1     Calc3   
 {    1.000000,   110.000000,  22.200000, 44.400000   },
 {    2.000000,   120.000000,  22.400000, 44.800000   },
 {    3.000000,   130.000000,  22.600000, 45.200000   },
 {    4.000000,   140.000000,  22.800000, 45.600000   },
 {    5.000000,   150.000000,  23.000000, 46.000000   },
 {    6.000000,   160.000000,  23.200000, 46.400000   },
 {    7.000000,   170.000000,  23.400000, 46.800000   },
 {    8.000000,   180.000000,  23.600000, 47.200000   },
 {    9.000000,   190.000000,  23.800000, 47.600000   },
 {   10.000000,   200.000000,  24.000000, 48.000000   },
 {   11.000000,   210.000000,  24.200000, 48.400000   },
 {   12.000000,   220.000000,  24.400000, 48.800000   },
 {   13.000000,   230.000000,  24.600000, 49.200000   },
 {   14.000000,   240.000000,  24.800000, 49.600000   },
 {   15.000000,   250.000000,  25.000000, 50.000000   },
 {   16.000000,   260.000000,  25.200000, 50.400000   },
 {   17.000000,   270.000000,  25.400000, 50.800000   },
 {   18.000000,   280.000000,  25.600000, 51.200000   },
 {   19.000000,   290.000000,  25.800000, 51.600000   },
 {   20.000000,   300.000000,  26.000000, 52.000000   },
 {   21.000000,   310.000000,  26.200000, 52.400000   },
 {   22.000000,   320.000000,  26.400000, 52.800000   },
 {   23.000000,   330.000000,  26.600000, 53.200000   },
 {   24.000000,   340.000000,  26.800000, 53.600000   },
 {   25.000000,   350.000000,  27.000000, 54.000000   },
 {   26.000000,   360.000000,  27.200000, 54.400000   },
 {   27.000000,   370.000000,  27.400000, 54.800000   },
 {   28.000000,   380.000000,  27.600000, 55.200000   },
 {   29.000000,   390.000000,  27.800000, 55.600000   },
 {   30.000000,   400.000000,  28.000000, 56.000000   },
};

static int CurrentResults = 0;

static void  
VerifyResults(double *Data, int NumberSignals, void *UserData)
{
    (void)UserData;
    (void)NumberSignals;


    double x        = Data[xOffset];
    double xCalc    = Data[xCalcOffset];
    double z        = Data[zOffset];
    double zCalc    = Data[zCalcOffset];

    if (!DoubleEqual(x, CorrectResults[CurrentResults][0])){
        if (Verbose){
            printf("FAILED VERIFICATION!\n");
        }        
        TestPass = false;
    }

    if (!DoubleEqual(z, CorrectResults[CurrentResults][1])){
        if (Verbose){
            printf("FAILED VERIFICATION!\n");
        }
        TestPass = false;
    }

    if (!DoubleEqual(xCalc, CorrectResults[CurrentResults][2])){
        if (Verbose){
            printf("FAILED VERIFICATION!\n");
        }        
        TestPass = false;
    }

    if (!DoubleEqual(zCalc, CorrectResults[CurrentResults][3])){
        if (Verbose){
            printf("FAILED VERIFICATION!\n");
        }
        TestPass = false;
    }

    CurrentResults++;
}

#endif


static double  fcnX(void *UserData)
{
    (void)UserData;

    static double x = 0.0;
    x = x + 1.0;
    return x;
}



static double  fcnY(void *UserData)
{
    (void)UserData;

    static double y = 10.0;
    y = y + 0.1;
    return y;
}


static double  fcnZ(void *UserData)
{
    (void)UserData;

    static double z = 100.0;
    z = z + 10.0;
    return z;
}



static double  Calc1(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;
    (void)cs;

    static double z = 22.0;
    z = z + 0.2;
    return z;
}



static double  Calc2(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;
    (void)cs;

    static double z = 33.0;
    z = z + 0.3;
    return z;
}



static double  Calc3(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;
    (void)cs;

    static double z = 44.0;
    z = z + 0.4;
    return z;
}


enum SignalIds {
    IdInvalid,
    IdFcnX,
    IdFcnY,
    IdFcnZ,
    IdCalc1,
    IdCalc2,
    IdCalc3,
};


bool UnitTestSelectDataOut2(void)
{
    int i;

    printf("Executing UnitTestSelectDataOut2\n");

    SmAddRawSignal(IdFcnX, "fcnX", fcnX, NULL, NULL);
    SmAddRawSignal(IdFcnY, "fcnY", fcnY, NULL, NULL);
    SmAddRawSignal(IdFcnZ, "fcnZ", fcnZ, NULL, NULL);


    INPUT_SIGNAL Calc1Input[] = {
        {IdFcnX, 0, 0},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL Calc2Input[] = {
        {IdFcnY, 0, 0},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL Calc3Input[] = {
        {IdFcnZ, 0, 0},
        INPUT_SIGNAL_END
    };
    
    SmAddCalcSignal(IdCalc1, "Calc1",  Calc1,  NULL, Calc1Input, NULL);
    SmAddCalcSignal(IdCalc2, "Calc2",  Calc2,  NULL, Calc2Input, NULL);
    SmAddCalcSignal(IdCalc3, "Calc3",  Calc3,  NULL, Calc3Input, NULL);

    SmInitCalculator();

    xOffset = SmGetDataReadyDataOffset(IdFcnX);
    xCalcOffset = SmGetDataReadyDataOffset(IdCalc1);
    zOffset = SmGetDataReadyDataOffset(IdFcnZ);
    zCalcOffset = SmGetDataReadyDataOffset(IdCalc3);

    if (Verbose)
        SmRegisterDataReadyCallback(PrintResults, NULL);

#if SM_VERIFY
    SmRegisterDataReadyCallback(VerifyResults, NULL);
#endif

    if (Verbose)
        SmPrintHeaders();

    for (i = 0; i<TEST_ITERATIONS; i++){
        SmRunCalculations(NULL);
    }

    return TestPass;
}



