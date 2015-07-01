#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>



#include "SegmentExecutionEngine.h"
#include "TaWinAssert.h"

#include "SegmentTest.h"


static float TestRampIterate = 0;


static SeStatus  
RampStartCallback(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData, 
                    void *Extension)
{
    //-------------------------------------------------
    TestRampData_t *RampData = (TestRampData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    (void)UserData;
    (void)Extension;
    (void)SegmentListHandle;

    sprintf(Buffer, "RampStartCallback %f degC @ %f degC/min", 
                RampData->TargetTempInC, 
                RampData->RateInCperMin);
    UtilTraceExecution(Buffer);

    TestRampIterate = RampData->TargetTempInC / RampData->RateInCperMin;

    return SeContinue;
}



static void  
RampStopCallback(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData,
                    void *Extension)
{
    //-------------------------------------------------
    TestRampData_t *RampData = (TestRampData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    (void)UserData;
    (void)Extension;
    (void)SegmentListHandle;

    sprintf(Buffer, "RampStopCallback %f deg C @ %f deg / min", 
                RampData->TargetTempInC, 
                RampData->RateInCperMin);

    UtilTraceExecution(Buffer);
    printf("%s\n", Buffer);
}



static SeStatus  
RampRunCallback(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData,
                    void *Extension)
{
    //-------------------------------------------------
    TestRampData_t *RampData = (TestRampData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    (void)UserData;
    (void)Extension;
    (void)SegmentListHandle;

    sprintf(Buffer, "RampRunCallback %f deg C @ %f deg / min", 
                RampData->TargetTempInC, 
                RampData->RateInCperMin);

    if (TestRampIterate > 0.0f){
        printf("+");
        TestRampIterate -= 1.0f;
        return SeContinue;
    }
    else{
        printf("\n");
        return SeSuccess;
    }
}



static void  
RampModifyCallback( SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData,
                    void *Extension)
{
    //-------------------------------------------------
    TestRampData_t *RampData = (TestRampData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    (void)UserData;
    (void)Extension;
    (void)SegmentListHandle;

    sprintf(Buffer, "RampModifyCallback %f degC @ %f degC/min", 
                RampData->TargetTempInC, 
                RampData->RateInCperMin);
    UtilTraceExecution(Buffer);
    printf("\n%s\n", Buffer);

    //
    //  Just change it for Test purposes.
    //
    TestRampIterate = RampData->TargetTempInC / RampData->RateInCperMin;
}



static SeStatus 
RampValidateCallback(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData)
{
    //-------------------------------------------------
    TestRampData_t *RampData = (TestRampData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    (void)SegmentListHandle;
    (void)UserData;

    if ((RampData->TargetTempInC <= -180) || (RampData->TargetTempInC > 800)){
        sprintf(Buffer, "RampValidateCallback FinalTemp %f degC - INVALID", 
                RampData->TargetTempInC);
        UtilTraceExecution(Buffer);
        printf("%s\n", Buffer);
		return SeInvalid;
    }
    else if ((RampData->RateInCperMin <= 0) || (RampData->RateInCperMin > 50)){
        sprintf(Buffer, "RampValidateCallback Rate %f degC/min - INVALID", 
                RampData->RateInCperMin);
        UtilTraceExecution(Buffer);
        printf("%s\n", Buffer);
		return SeInvalid;
    }
    else{
        sprintf(Buffer, "RampValidateCallback %f degC @ %f degC/min - VALID", 
                RampData->TargetTempInC, 
                RampData->RateInCperMin);
        UtilTraceExecution(Buffer);
        return SeSuccess;
    }
}



static void  
RampToStringCallback(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData, 
                        void *Extension,
                        char *StringBuffer, 
                        int *MaxStringBufferLength)
{
    //-------------------------------------------------
    TestRampData_t *RampData = (TestRampData_t *)Data;
    //-------------------------------------------------

    (void)Extension;
    (void)SegmentListHandle;
    (void)UserData;

    *MaxStringBufferLength = snprintf(StringBuffer, *MaxStringBufferLength, 
                        "RAMP %f degC @ %f degC/min", 
                        RampData->TargetTempInC, 
                        RampData->RateInCperMin);

    UtilTraceExecution(StringBuffer);
}



static char *RampUserData = "RAMP-USER-DATA";


void
RegisterRampSegment(ENGINE_HANDLE Engine)
{
    SegmentCommand_t Command = {

        TestRamp,                   //  From CiiMessageData.<Type>.SubCommand 

        ScfDefault,                 //  SegmentFlags
        0,                          //  Custom flags
        sizeof(TestRampData_t),     //  Expected data length to be passed in.
        RampUserData,               //  Up to you

        RampStartCallback,          //  Start - Must exist! 
        RampToStringCallback,       //  ToString - Must exist! 
        RampRunCallback,            //  Run - Can be NULL if the Segment is instant
        RampStopCallback,           //  Stop - Can be NULL if the Segment is instant
        RampModifyCallback,         //  Modify - Can be NULL 
        RampValidateCallback,       //  Validate - Can be NULL 
        NULL,                       //  Setup - Can be NULL 
        NULL,                       //  Cleanup - Can be NULL
        NULL,                       //  Lock - Can be NULL
        NULL,                       //  Unlock - Can be NULL
        NULL,                       //  InitSearch
        NULL,                       //  ModifyIndexForSearch
    };

    SeRegisterSegmentCommand(Engine, &Command);
}
