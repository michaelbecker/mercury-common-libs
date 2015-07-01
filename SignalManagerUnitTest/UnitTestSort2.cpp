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

#define TEST_ITERATIONS 1


static double CorrectResults[TEST_ITERATIONS][11] = 
{

{   10.100000,    7.700000,    1.000000,   17.800000,    1.100000,    1.320000,    1.716000,    1.733160,    1.767823,    1.820858,   20.620858,  },

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


static double  fcnZ(void *UserData)
{
    (void)UserData;

    static double x = 7.0;
    x = x + 0.7;
    return x;
}



static double  fcnA ( SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double ans = SmGetInputResult(cs, 0, 0);
    ans *= 1.1;
   
    return ans;
}


static double  fcnB ( SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double a = SmGetInputResult(cs, 0, 0);
   
    return a * 1.2;
}


static double  fcnC ( SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double b = SmGetInputResult(cs, 0, 0);
   
    return b * 1.3;
}



static double  fcnD ( SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double e = SmGetInputResult(cs, 0, 0);
   
    return e * 1.03;
}



static double  fcnE ( SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double f = SmGetInputResult(cs, 0, 0);
   
    return f * 1.02;
}


static double  fcnF ( SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double c = SmGetInputResult(cs, 0, 0);
   
    return c * 1.01;
}


static double  fcnG ( SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double y = SmGetInputResult(cs, 0, 0);
    double z = SmGetInputResult(cs, 0, 1);
   
    return y + z;
}



static double  fcnH ( SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    double d = SmGetInputResult(cs, 0, 0);
    double x = SmGetInputResult(cs, 0, 1);
    double y = SmGetInputResult(cs, 0, 2);
    double z = SmGetInputResult(cs, 0, 3);
   
    return d + x + y + z;
}



enum SignalIds {
    IdInvalid,
    IdfcnX,
    IdfcnY,
    IdfcnZ,
    IdfcnA,
    IdfcnB,
    IdfcnC,
    IdfcnD,
    IdfcnE,
    IdfcnF,
    IdfcnG,
    IdfcnH,
};


bool UnitTestSort2(void)
{
    int i;

    printf("Executing UnitTestSort2\n");

    INPUT_SIGNAL fcnAInput[] = {
        {IdfcnX, 0, 0},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL fcnBInput[] = {
        {IdfcnA, 0, 0},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL fcnCInput[] = {
        {IdfcnB, 0, 0},
        INPUT_SIGNAL_END
    };
    
    INPUT_SIGNAL fcnDInput[] = {
        {IdfcnE, 0, 0},
        INPUT_SIGNAL_END
    };
    
    INPUT_SIGNAL fcnEInput[] = {
        {IdfcnF, 0, 0},
        INPUT_SIGNAL_END
    };
    
    INPUT_SIGNAL fcnFInput[] = {
        {IdfcnC, 0, 0},
        INPUT_SIGNAL_END
    };
    
    INPUT_SIGNAL fcnGInput[] = {
        {IdfcnY, 0, 0},
        {IdfcnZ, 0, 0},
        INPUT_SIGNAL_END
    };
    
    INPUT_SIGNAL fcnHInput[] = {
        {IdfcnD, 0, 0},
        {IdfcnX, 0, 0},
        {IdfcnY, 0, 0},
        {IdfcnZ, 0, 0},
        INPUT_SIGNAL_END
    };


    SmAddCalcSignal(IdfcnD, "fcnD", fcnD,  NULL, fcnDInput, NULL);
    SmAddCalcSignal(IdfcnC, "fcnC", fcnC,  NULL, fcnCInput, NULL);
    SmAddCalcSignal(IdfcnB, "fcnB", fcnB,  NULL, fcnBInput, NULL);
    SmAddCalcSignal(IdfcnA, "fcnA", fcnA,  NULL, fcnAInput, NULL);
    SmAddCalcSignal(IdfcnH, "fcnH", fcnH,  NULL, fcnHInput, NULL);
    SmAddCalcSignal(IdfcnG, "fcnG", fcnG,  NULL, fcnGInput, NULL);
    SmAddCalcSignal(IdfcnF, "fcnF", fcnF,  NULL, fcnFInput, NULL);
    SmAddCalcSignal(IdfcnE, "fcnE", fcnE,  NULL, fcnEInput, NULL);

    SmAddRawSignal(IdfcnY, "fcnY", fcnY, NULL, &fcnYUserData);
    SmAddRawSignal(IdfcnZ, "fcnZ", fcnZ, NULL, NULL);
    SmAddRawSignal(IdfcnX, "fcnX", fcnX, NULL, NULL);

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


