#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>




#include "TaWinAssert.h"
#include "SegmentExecutionEngine.h"
#include "SegmentTest.h"


bool RunEnded = false;
bool RunTerminated = false;
int CurrentSegment = -1;


SE_SEGMENT_LIST_HANDLE ContinueHandle = NULL;
SE_SEGMENT_LIST_HANDLE ContinueHandleInProgress = NULL;



void  EndCallback(void *UserData)
{
    UtilTraceExecution("EndCallback");

	if (ContinueHandle){
		UtilTraceExecution("Starting ContinueHandle from EndCallback");
		SeStartSegmentListFromEndCallback(ContinueHandle);
		ContinueHandleInProgress = ContinueHandle;
		ContinueHandle = NULL;
	}
	else{
		*(bool *)UserData = true;
	}
}


void  TerminateCallback(void *UserData)
{
    *(bool *)UserData = true;
    UtilTraceExecution("TerminateCallback");
}


void  SegmentChangeCallback(   int CurrentSegmentIndex,        //  Zero based index into the Segment List.
                                        SegmentBits_t *RawSegment,      //  Actual pointer to the data sent by Trios
                                        unsigned int RawSegmentLength,  
                                        char *String,                   //  Translation from the SegmentCommand.ToStringCallback()
                                        void *UserData)                 //  Whatever you passed in as SegmentChangedUserData
{
    char Buffer[128];
    *(int *)UserData = CurrentSegmentIndex;
    unsigned char *BufPtr;

    BufPtr = (unsigned char *)RawSegment;

    sprintf(Buffer, "Segment %d", CurrentSegmentIndex + 1);
    printf("Segment %d - ", CurrentSegmentIndex + 1);
    UtilTraceExecution(Buffer);

    printf("%s - ", String);
    printf("[%d - ", RawSegmentLength);
    for (unsigned int i = 0; i<RawSegmentLength; i++){
        printf("%02x", BufPtr[i]);
    }
    printf(" - ");
    for (unsigned int i = 0; i<RawSegmentLength; i++){
        if (isalnum(BufPtr[i])){
            printf("%c", BufPtr[i]);
        }
        else{
            printf(".");
        }
    }
    printf("]");

    printf("\n");
}


void 
RegisterStaticCommands(ENGINE_HANDLE Engine)
{
    StaticCommands_t Commands = {

        EndCallback,            &RunEnded,
        TerminateCallback,      &RunTerminated,
        SegmentChangeCallback,  &CurrentSegment
    };

    SeRegisterAllStaticCommands(Engine, &Commands);
}


void
ResetStaticCommandTestData(void)
{
    RunEnded = false;
    RunTerminated = false;
    CurrentSegment = -1;
}

