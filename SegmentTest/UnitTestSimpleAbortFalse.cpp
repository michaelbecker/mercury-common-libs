#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"

static char *ExpectedTrace[] = {
    "AbortValidateCallback 10 VALID",
    "IsoValidateCallback 12.000000 VALID",
    "ABORT when Count = 10",
    "Segment 1",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 10 (cur 1000)",
    "ISO 12.000000",
    "Segment 2",
    "IsoStartCallback 12.000000",
    "AbortRunCallback at 10 (cur 999)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 998)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 997)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 996)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 995)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 994)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 993)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 992)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 991)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 990)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 989)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 988)",
    "IsoRunCallback 12.000000",
    "AbortRunCallback at 10 (cur 987)",
    "IsoRunCallback 12.000000",
    "EndCallback",
};


bool 
UnitTestSimpleAbortFalse(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 2;
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
    AbortData.AbortCount = 10;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestAbort, 
                                                 Id++,
                                                (unsigned char *)&AbortData, 
                                                sizeof(AbortData),
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 12;
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

    
    UnitTestSetAbortCountDown(1000);

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
