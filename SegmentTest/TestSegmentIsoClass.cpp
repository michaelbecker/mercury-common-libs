#include <stdio.h>


#include "TestSegmentIsoClass.h"
#include "TestSegments.h"
#include "TaWinAssert.h"
#include "SegmentTest.h"



TestSegmentIsoClass::TestSegmentIsoClass(ENGINE_HANDLE hEngine)
    :ISegmentSimple(hEngine, TestIso2, 0, sizeof(TestIsoData_t), "ISO2CLASS"),
    TestIsoIterate(0.0f),
    Verbose(false)
{
}



SeStatus 
TestSegmentIsoClass::StartCallback(unsigned char *Data)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    sprintf(Buffer, "Iso2ClassStartCallback %f", IsoData->TimeInSeconds);
    UtilTraceExecution(Buffer);

    TestIsoIterate = IsoData->TimeInSeconds;
    return SeContinue;
}



void 
TestSegmentIsoClass::ToStringCallback(unsigned char *Data,
                                        char *StringBuffer,
                                        int *MaxStringBufferLength)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    //-------------------------------------------------

    *MaxStringBufferLength = snprintf(StringBuffer, *MaxStringBufferLength,
                                       "%s %f",
                                       GetName(), IsoData->TimeInSeconds);

    UtilTraceExecution(StringBuffer);
}



SeStatus 
TestSegmentIsoClass::ValidateCallback(unsigned char *Data)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    if (IsoData->TimeInSeconds <= 0){

        sprintf(Buffer, "Iso2ClassValidateCallback %f INVALID", IsoData->TimeInSeconds);
        UtilTraceExecution(Buffer);
        printf("%s\n", Buffer);

        return SeInvalid;
    }
    else{
        sprintf(Buffer, "Iso2ClassValidateCallback %f VALID", IsoData->TimeInSeconds);
        UtilTraceExecution(Buffer);
        return SeSuccess;
    }
}



void 
TestSegmentIsoClass::StopCallback(unsigned char *Data)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    sprintf(Buffer, "Iso2ClassStopCallback %f", IsoData->TimeInSeconds);
    UtilTraceExecution(Buffer);

    printf("%s\n", Buffer);
}



SeStatus 
TestSegmentIsoClass::RunCallback(unsigned char *Data)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    if (Verbose){
        sprintf(Buffer, "Iso2ClassRunCallback %f", IsoData->TimeInSeconds);
        UtilTraceExecution(Buffer);
    }

    if (TestIsoIterate > 0.0f){
        printf(".");
        TestIsoIterate -= 1.0f;
        return SeContinue;
    }
    else{
        printf("\n");
        return SeSuccess;
    }
}



void 
TestSegmentIsoClass::ModifyCallback(unsigned char *Data)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    sprintf(Buffer, "Iso2ClassModifyCallback (New Time %f)", IsoData->TimeInSeconds);
    UtilTraceExecution(Buffer);
    printf("\n%s\n", Buffer);

    //
    //  Just change it for testing purposes.
    //
    TestIsoIterate = IsoData->TimeInSeconds;
}

