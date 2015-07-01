#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"

static char *ExpectedTrace[] = {
    "IsoValidateCallback 10.000000 VALID",
    "AbortValidateCallback 95 VALID",
    "ISO 10.000000",
    "Segment 1",
    "IsoStartCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "IsoRunCallback 10.000000",
    "ABORT when Count = 95",
    "Segment 2",
    "AbortStartCallback Condition TRUE - Skipping next segment (10 <= 95)",
    "EndCallback",
};


bool 
UnitTestAbortRepeatTrue(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 3;
    int Id = 0x100;
    TestIsoData_t Time;
    TestAbortData_t AbortData;
    CsRepeatData_t RepeatData;

    SegmentBits_t *SegmentArray[NumSegments];
    int SegmentLength[NumSegments];
    int BufferLength;
    SeStatus Status;
    int SegmentIndex;
    int rc;
    int idx = 0;
    //------------------------------------

    //--------------------------------------------------------------
    Time.TimeInSeconds = 10;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso,  
                                                Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    AbortData.AbortCount = 95;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestAbort, 
                                                 Id++,
                                                (unsigned char *)&AbortData, 
                                                sizeof(AbortData),
                                                &SegmentLength[idx]);
    idx++;


    //--------------------------------------------------------------
    RepeatData.Index = 0;
    RepeatData.Count = 10;
    SegmentArray[idx] = UtilCreateSegmentMessage( CsRepeat, 
                                                 Id++,
                                                (unsigned char *)&RepeatData, 
                                                sizeof(RepeatData),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------


    unsigned char *Buffer = UtilCreateSegmentMessageList(  SegmentArray,
                                                    SegmentLength,
                                                    NumSegments,
                                                    &BufferLength);

    SE_SEGMENT_LIST_HANDLE Handle = SeCreateSegmentList(Engine, 
                                                        Buffer, 
                                                        BufferLength, 
                                                        &Status, 
                                                        &SegmentIndex);
    if (!Handle){
        printf("Failed SeCreateSegmentList(), Status = %d, BadSegmentIndex = %d\n",
                Status, SegmentIndex);
        return false;
    }

    UtilFreeSegmentMessageBuffers( SegmentArray,
                            NumSegments,
                            Buffer);

    
    UnitTestSetAbortCountDown(10);

    rc = SeStartSegmentList(Handle);
    if (rc != SeSuccess){
        return false;
    }


    //
    //  Wait for the test to be done...
    //
    while (!RunEnded && !RunTerminated){
        sleep(1);
    }
    if (RunEnded){
        printf("Unit test ended.\n");
    }
    else if (RunTerminated){
        printf("Unit test Terminated!\n");
        return false;
    }

	SeDestroySegmentList(Handle);

    if (CompareTraceExecution(ExpectedTrace, sizeof(ExpectedTrace) / sizeof(char *))){
        return true;
    }
    else{
        return false;
    }
}
