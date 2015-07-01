#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>




#include "SegmentExecutionEngine.h"
#include "TaWinAssert.h"

#include "SegmentTest.h"


static float TestIsoIterate = 0;
static bool Verbose = true;


void SetIsoVerbose(bool _verbose)
{
    Verbose = _verbose;
}



static SeStatus  
IsoStartCallback(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData,
                    void *Extension)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    (void)UserData;
    (void)Extension;
    (void)SegmentListHandle;

    sprintf(Buffer, "IsoStartCallback %f", IsoData->TimeInSeconds);
    UtilTraceExecution(Buffer);

    TestIsoIterate = IsoData->TimeInSeconds;
    return SeContinue;
}



static void  
IsoStopCallback(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData,
                    void *Extension)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    (void)UserData;
    (void)Extension;
    (void)SegmentListHandle;

    sprintf(Buffer, "IsoStopCallback %f", IsoData->TimeInSeconds);
    UtilTraceExecution(Buffer);

    printf("%s\n", Buffer);
}



static SeStatus  
IsoRunCallback( SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                unsigned char *Data, 
                void *UserData,
                void *Extension)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    (void)UserData;
    (void)Extension;
    (void)SegmentListHandle;

    if (Verbose){
        sprintf(Buffer, "IsoRunCallback %f", IsoData->TimeInSeconds);
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



static void  
IsoModifyCallback(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData,
                    void *Extension)
{
    //-------------------------------------------------
    (void)Extension;
    (void)UserData;
    (void)SegmentListHandle;

    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    sprintf(Buffer, "IsoModifyCallback (New Time %f)", IsoData->TimeInSeconds);
    UtilTraceExecution(Buffer);
    printf("\n%s\n", Buffer);

    //
    //  Just change it for testing purposes.
    //
    TestIsoIterate = IsoData->TimeInSeconds;
}



static SeStatus 
IsoValidateCallback(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    char Buffer[128];
    //-------------------------------------------------

    (void)SegmentListHandle;
    (void)UserData;

    if (IsoData->TimeInSeconds <= 0){

        sprintf(Buffer, "IsoValidateCallback %f INVALID", IsoData->TimeInSeconds);
        UtilTraceExecution(Buffer);
        printf("%s\n", Buffer);

		return SeInvalid;
    }
    else{
        sprintf(Buffer, "IsoValidateCallback %f VALID", IsoData->TimeInSeconds);
        UtilTraceExecution(Buffer);
        return SeSuccess;
    }
}



static void  
IsoToStringCallback(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData, 
                        void *Extension,
                        char *StringBuffer, 
                        int *MaxStringBufferLength)
{
    //-------------------------------------------------
    TestIsoData_t *IsoData = (TestIsoData_t *)Data;
    //-------------------------------------------------

    (void)Extension;
    (void)SegmentListHandle;
    (void)UserData;

    *MaxStringBufferLength = snprintf(StringBuffer, *MaxStringBufferLength, 
                "ISO %f", 
                IsoData->TimeInSeconds);

    UtilTraceExecution(StringBuffer);
}



static char *IsoUserData = "ISO-USER-DATA";


void
RegisterIsoSegment(ENGINE_HANDLE Engine)
{
    SegmentCommand_t Command = {

        TestIso,                //  From CiiMessageData.<Type>.SubCommand     

        ScfDefault,             //  SegmentFlags
        0,                      //  Custom flags    
        sizeof(TestIsoData_t),  //  Expected data length to be passed in.    
        IsoUserData,            //  Up to you    

        IsoStartCallback,       //  Start - Must exist!     
        IsoToStringCallback,    //  ToString - Must exist!     
        IsoRunCallback,         //  Run - Can be NULL if the Segment is instant    
        IsoStopCallback,        //  Stop - Can be NULL if the Segment is instant    
        IsoModifyCallback,      //  Modify - Can be NULL     
        IsoValidateCallback,    //  Validate - Can be NULL     
        NULL,                   //  Setup - Can be NULL     
        NULL,                   //  Cleanup - Can be NULL    
        NULL,                   //  Lock - Can be NULL    
        NULL,                   //  Unlock - Can be NULL        
        NULL,                   //  InitSearch
        NULL,                   //  ModifyIndexForSearch
    };

    SeRegisterSegmentCommand(Engine, &Command);
}
