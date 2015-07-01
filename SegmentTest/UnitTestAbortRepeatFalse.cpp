#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"

static char *ExpectedTrace[] = {
    "IsoValidateCallback 3.000000 VALID",
    "AbortValidateCallback 95 VALID",
    "ISO 3.000000",
    "Segment 1",
    "IsoStartCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "ABORT when Count = 95",
    "Segment 2",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 500)",
    "Segment 3",
    "ISO 3.000000",
    "Segment 1",
    "IsoStartCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "ABORT when Count = 95",
    "Segment 2",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 499)",
    "Segment 3",
    "ISO 3.000000",
    "Segment 1",
    "IsoStartCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "ABORT when Count = 95",
    "Segment 2",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 498)",
    "Segment 3",
    "ISO 3.000000",
    "Segment 1",
    "IsoStartCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "ABORT when Count = 95",
    "Segment 2",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 497)",
    "Segment 3",
    "ISO 3.000000",
    "Segment 1",
    "IsoStartCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "ABORT when Count = 95",
    "Segment 2",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 496)",
    "Segment 3",
    "ISO 3.000000",
    "Segment 1",
    "IsoStartCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "IsoRunCallback 3.000000",
    "ABORT when Count = 95",
    "Segment 2",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 495)",
    "Segment 3",
    "EndCallback",
};
bool 
UnitTestAbortRepeatFalse(ENGINE_HANDLE Engine)
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
    Time.TimeInSeconds = 3;
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
    RepeatData.Count = 5;
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

    
    UnitTestSetAbortCountDown(500);

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
