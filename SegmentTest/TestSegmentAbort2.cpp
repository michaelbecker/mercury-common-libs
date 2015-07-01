#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>




#include "SegmentExecutionEngine.h"
#include "TaWinAssert.h"

#include "SegmentTest.h"


static int AbortCountDown = 0;

void
UnitTestSetAbort2CountDown(int InitialCount)
{
    AbortCountDown = InitialCount;
}



static SeStatus  
AbortStartCallback( SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData,
                    void *Extension)
{
    //------------------------------------------------------
    TestAbortData_t *AbortData = (TestAbortData_t *)Data;
    char Buffer[128];
    //------------------------------------------------------

    (void)UserData;
    (void)Extension;
    (void)SegmentListHandle;

    if (AbortCountDown > AbortData->AbortCount){
        sprintf(Buffer, "Abort2StartCallback Condition FALSE - Attaching to next segment - abort at %d (cur %d)", 
                        AbortData->AbortCount, AbortCountDown);
        printf("%s\n", Buffer);
        UtilTraceExecution(Buffer);
        AbortCountDown -= 1;
        return SeAbortIfConditionAttach;
    }
    else{
        sprintf(Buffer, "Abort2StartCallback Condition TRUE - Skipping next segment (%d <= %d)", 
                        AbortCountDown, AbortData->AbortCount);
        printf("%s\n", Buffer);
        UtilTraceExecution(Buffer);
        return SeAbortIfConditionSkip;
    }
}



static SeStatus  
AbortRunCallback(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData,
                    void *Extension)
{
    //------------------------------------------------------
    TestAbortData_t *AbortData = (TestAbortData_t *)Data;
    char Buffer[128];
    //------------------------------------------------------

    (void)UserData;
    (void)Extension;
    (void)SegmentListHandle;

    if (AbortCountDown > AbortData->AbortCount){
        sprintf(Buffer, "Abort2RunCallback at %d (cur %d)", AbortData->AbortCount, AbortCountDown);
        UtilTraceExecution(Buffer);
        printf("a");
        AbortCountDown -= 1;
        return SeAbortIfConditionFalse;
    }
    else{
        sprintf(Buffer, "Abort2RunCallback Condition TRUE - Aborting next segment (%d <= %d)", 
                        AbortCountDown, AbortData->AbortCount);
        printf("\n%s\n", Buffer);
        UtilTraceExecution(Buffer);
        return SeAbortIfConditionTrue;
    }
}



static SeStatus 
AbortValidateCallback(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData)
{
    //------------------------------------------------------
    TestAbortData_t *AbortData = (TestAbortData_t *)Data;
    char Buffer[128];
    //------------------------------------------------------

    (void)SegmentListHandle;
    (void)UserData;

    if (AbortData->AbortCount < 0){
        sprintf(Buffer, "Abort2ValidateCallback %d INVALID", AbortData->AbortCount);
        UtilTraceExecution(Buffer);
        printf("%s\n", Buffer);
		return SeInvalid;
    }
    else{
        sprintf(Buffer, "Abort2ValidateCallback %d VALID", AbortData->AbortCount);
        UtilTraceExecution(Buffer);
        return SeSuccess;
    }
}



static void  
AbortToStringCallback(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData, 
                        void *Extension,
                        char *StringBuffer, 
                        int *MaxStringBufferLength)
{
    //------------------------------------------------------
    TestAbortData_t *AbortData = (TestAbortData_t *)Data;
    //------------------------------------------------------

    (void)Extension;
    (void)SegmentListHandle;
    (void)UserData;

    *MaxStringBufferLength = snprintf(StringBuffer, *MaxStringBufferLength, 
                "ABORT2 when Count = %d", 
                AbortData->AbortCount);

    UtilTraceExecution(StringBuffer);
}



static char *AbortUserData = "ABORT2-USER-DATA";


void
RegisterAbort2Segment(ENGINE_HANDLE Engine)
{
    SegmentCommand_t Command = {

        TestAbort2,             //  From CiiMessageData.<Type>.SubCommand     

        ScfAbortConditional,    //  SegmentFlags
        0,                      //  Custom flags    
        sizeof(TestAbortData_t),//  Expected data length to be passed in.
        AbortUserData,          //  Up to you    

        AbortStartCallback,     //  Start - Must exist!     
        AbortToStringCallback,  //  ToString - Must exist!     
        AbortRunCallback,       //  Run - Can be NULL if the Segment is instant    
        NULL,                   //  Stop - Can be NULL if the Segment is instant    
        NULL,                   //  Modify - Can be NULL     
        AbortValidateCallback,  //  Validate - Can be NULL     
        NULL,                   //  Setup - Can be NULL     
        NULL,                   //  Cleanup - Can be NULL    
        NULL,                   //  Lock - Can be NULL    
        NULL,                   //  Unlock - Can be NULL        
        NULL,                   //  InitSearch
        NULL,                   //  ModifyIndexForSearch
    };

    SeRegisterSegmentCommand(Engine, &Command);
}
