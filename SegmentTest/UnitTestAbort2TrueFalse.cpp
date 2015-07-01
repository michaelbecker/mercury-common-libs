#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"



static char *ExpectedTrace[] = {
    "AbortValidateCallback 95 VALID",
    "Abort2ValidateCallback 95 VALID",
    "IsoValidateCallback 1000.000000 VALID",
    "ABORT when Count = 95",
    "Segment 1",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 105)",
    "ABORT2 when Count = 95",
    "Segment 2",
    "Abort2StartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 1000)",
    "ISO 1000.000000",
    "Segment 3",
    "IsoStartCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 999)",
    "AbortRunCallback at 95 (cur 104)",
    "IsoRunCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 998)",
    "AbortRunCallback at 95 (cur 103)",
    "IsoRunCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 997)",
    "AbortRunCallback at 95 (cur 102)",
    "IsoRunCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 996)",
    "AbortRunCallback at 95 (cur 101)",
    "IsoRunCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 995)",
    "AbortRunCallback at 95 (cur 100)",
    "IsoRunCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 994)",
    "AbortRunCallback at 95 (cur 99)",
    "IsoRunCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 993)",
    "AbortRunCallback at 95 (cur 98)",
    "IsoRunCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 992)",
    "AbortRunCallback at 95 (cur 97)",
    "IsoRunCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 991)",
    "AbortRunCallback at 95 (cur 96)",
    "IsoRunCallback 1000.000000",
    "Abort2RunCallback at 95 (cur 990)",
    "AbortRunCallback Condition TRUE - Aborting next segment (95 <= 95)",
    "IsoStopCallback 1000.000000",
    "EndCallback",
};



bool 
UnitTestAbort2TrueFalse(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 3;
    int Id = 0x100;
    TestIsoData_t Time;
    TestAbortData_t AbortData;

    SegmentBits_t *SegmentArray[NumSegments];
    int SegmentLength[NumSegments];
    int BufferLength;
    SeStatus Status;
    int SegmentIndex;
    int rc;
    int idx = 0;
    //------------------------------------

    //--------------------------------------------------------------
    AbortData.AbortCount = 95;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestAbort, 
                                                 Id++,
                                                (unsigned char *)&AbortData, 
                                                sizeof(AbortData),
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    AbortData.AbortCount = 95;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestAbort2, 
                                                 Id++,
                                                (unsigned char *)&AbortData, 
                                                sizeof(AbortData),
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 1000;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
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

    
    UnitTestSetAbortCountDown(105);
    UnitTestSetAbort2CountDown(1000);

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
