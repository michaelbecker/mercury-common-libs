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

static int yOffset;
static int yCalcOffset;


#if SM_VERIFY
static double CorrectResults[TEST_ITERATIONS][12] = 
{

 {   10.100000,   33.300000  },
 {   10.200000,   33.600000  },
 {   10.300000,   33.900000  },
 {   10.400000,   34.200000  },
 {   10.500000,   34.500000  },
 {   10.600000,   34.800000  },
 {   10.700000,   35.100000  },
 {   10.800000,   35.400000  },
 {   10.900000,   35.700000  },
 {   11.000000,   36.000000  },
 {   11.100000,   36.300000  },
 {   11.200000,   36.600000  },
 {   11.300000,   36.900000  },
 {   11.400000,   37.200000  },
 {   11.500000,   37.500000  },
 {   11.600000,   37.800000  },
 {   11.700000,   38.100000  },
 {   11.800000,   38.400000  },
 {   11.900000,   38.700000  },
 {   12.000000,   39.000000  },
 {   12.100000,   39.300000  },
 {   12.200000,   39.600000  },
 {   12.300000,   39.900000  },
 {   12.400000,   40.200000  },
 {   12.500000,   40.500000  },
 {   12.600000,   40.800000  },
 {   12.700000,   41.100000  },
 {   12.800000,   41.400000  },
 {   12.900000,   41.700000  },
 {   13.000000,   42.000000  },
};

static int CurrentResults = 0;

static void  
VerifyResults(double *Data, int NumberSignals, void *UserData)
{
    (void)UserData;
    (void)NumberSignals;

    double y        = Data[yOffset];
    double yCalc    = Data[yCalcOffset];

    if (!DoubleEqual(y, CorrectResults[CurrentResults][0])){
        if (Verbose){
            printf("FAILED VERIFICATION!\n");
        }        
        TestPass = false;
    }

    if (!DoubleEqual(yCalc, CorrectResults[CurrentResults][1])){
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


bool UnitTestSelectDataOut1(void)
{
    int i;

    printf("Executing UnitTestSelectDataOut1\n");

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

    yOffset = SmGetDataReadyDataOffset(IdFcnY);
    yCalcOffset = SmGetDataReadyDataOffset(IdCalc2);

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



