#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>



#include "SegmentExecutionEngine.h"
#include "TaWinAssert.h"

#include "SegmentTest.h"


//
//  This is a pretend repeat until segment.  In it we look for a 
//  heat flow less than what was requested to break out of the 
//  loop.
//
static volatile float gCurrentHeatflow = 0;

void
UnitTestSetHeatflow(float hf)
{
    gCurrentHeatflow = hf;
}



static SeStatus  
RepeatUntilStartCallback(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                            unsigned char *Data, 
                            void *UserData, 
                            void *Extension)
{
    //---------------------------------------
    (void)UserData;
    (void)Extension;
    TestRepeatUntilData_t *RepeatUntilData;
    //---------------------------------------

    //
    //  Test code, 
    //
    if (gCurrentHeatflow > 0.0){
        gCurrentHeatflow -= 1.0;
    }

    RepeatUntilData = (TestRepeatUntilData_t *)Data;

    //
    //  looping == changing an index
    //
    if (gCurrentHeatflow > RepeatUntilData->Heatflow){
        //
        //  Change the index of the segment list.
        //
        SeSetIndexForBranch(SegmentListHandle, RepeatUntilData->Index);
        //
        //  And let the engine know we did it.
        //
        return SeActiveSegmentIndexModified;
    }
    else{
        //
        //  We are done, move on to the next segment.
        //
        return SeSuccess;
    }
}



static SeStatus 
RepeatUntilValidateCallback(SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                            unsigned char *Data, 
                            void *UserData)
{
    //-----------------------------------------------------
    (void)UserData;
    TestRepeatUntilData_t *RepeatUntilData;
    char Buffer[128];
    //-----------------------------------------------------

    RepeatUntilData = (TestRepeatUntilData_t *)Data;

    unsigned int TotalNumSegments = SeGetTotaltNumSegments(SegmentListHandle);

    //
    //  We shouldn't walk off the end.
    //
    if (RepeatUntilData->Index >= TotalNumSegments){
        sprintf(Buffer, "RepeatUntilValidateCallback branching off list! branch %d (list is %d)", 
                RepeatUntilData->Index, TotalNumSegments);
        UtilTraceExecution(Buffer);
        printf("%s\n", Buffer);
		return SeInvalid;
    }

    int MySegmentIndex = SeGetCurrentSegmentIndex(SegmentListHandle);

    //
    //  Prevent infinite loops and looping "ahead" of ourselves.
    //
    if ((unsigned int)MySegmentIndex <= RepeatUntilData->Index){
        sprintf(Buffer, "RepeatUntilValidateCallback branching invalid! branch from %d to %d", 
                MySegmentIndex, RepeatUntilData->Index);
        UtilTraceExecution(Buffer);
        printf("%s\n", Buffer);
		return SeInvalid;
    }

    sprintf(Buffer, "RepeatUntilValidateCallback, Repeat %d until Heatflow < %f", 
            RepeatUntilData->Index + 1, 
            RepeatUntilData->Heatflow);
    UtilTraceExecution(Buffer);
	return SeSuccess;
}



static void  
RepeatUntilToStringCallback(SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                            unsigned char *Data, 
                            void *UserData, 
                            void *Extension,
                            char *StringBuffer, 
                            int *MaxStringBufferLength)
{
    //-------------------------------------------------
    (void)Extension;
    (void)SegmentListHandle;
    (void)UserData;
    TestRepeatUntilData_t *RepeatUntilData;
    //-------------------------------------------------

    RepeatUntilData = (TestRepeatUntilData_t *)Data;

    *MaxStringBufferLength = snprintf(StringBuffer, *MaxStringBufferLength, 
                        "REPEAT %d UNTIL HEATFLOW < %f", 
                        RepeatUntilData->Index + 1, 
                        RepeatUntilData->Heatflow);

    UtilTraceExecution(StringBuffer);
}



static char *RepeatUntilUserData = "REPEAT-UNTIL-USER-DATA";


void
RegisterRepeatUntilSegment(ENGINE_HANDLE Engine)
{
    SegmentCommand_t Command = {

        TestRepeatUntil,                //  From CiiMessageData.<Type>.SubCommand 

        ScfRepeat,                      //  SegmentFlags
        0,                              //  Custom flags
        sizeof(TestRepeatUntilData_t),  //  Expected data length to be passed in.
        RepeatUntilUserData,            //  Up to you

        RepeatUntilStartCallback,       //  Start - Must exist! 
        RepeatUntilToStringCallback,    //  ToString - Must exist! 
        NULL,                           //  Run - Can be NULL if the Segment is instant
        NULL,                           //  Stop - Can be NULL if the Segment is instant
        NULL,                           //  Modify - Can be NULL 
        RepeatUntilValidateCallback,    //  Validate - Can be NULL 
        NULL,                           //  Setup - Can be NULL 
        NULL,                           //  Cleanup - Can be NULL
        NULL,                           //  Lock - Can be NULL
        NULL,                           //  Unlock - Can be NULL
        NULL,                           //  InitSearch
        NULL,                           //  ModifyIndexForSearch
    };

    SeRegisterSegmentCommand(Engine, &Command);
}