#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"

static char *ExpectedTrace[] = {
    "AbortValidateCallback 95 VALID",
    "Abort2ValidateCallback 95 VALID",
    "IsoValidateCallback 20.000000 VALID",
    "ABORT when Count = 95",
    "Segment 1",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 1000)",
    "ABORT2 when Count = 95",
    "Segment 2",
    "Abort2StartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 1500)",
    "ISO 20.000000",
    "Segment 3",
    "IsoStartCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1499)",
    "AbortRunCallback at 95 (cur 999)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1498)",
    "AbortRunCallback at 95 (cur 998)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1497)",
    "AbortRunCallback at 95 (cur 997)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1496)",
    "AbortRunCallback at 95 (cur 996)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1495)",
    "AbortRunCallback at 95 (cur 995)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1494)",
    "AbortRunCallback at 95 (cur 994)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1493)",
    "AbortRunCallback at 95 (cur 993)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1492)",
    "AbortRunCallback at 95 (cur 992)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1491)",
    "AbortRunCallback at 95 (cur 991)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1490)",
    "AbortRunCallback at 95 (cur 990)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1489)",
    "AbortRunCallback at 95 (cur 989)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1488)",
    "AbortRunCallback at 95 (cur 988)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1487)",
    "AbortRunCallback at 95 (cur 987)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1486)",
    "AbortRunCallback at 95 (cur 986)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1485)",
    "AbortRunCallback at 95 (cur 985)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1484)",
    "AbortRunCallback at 95 (cur 984)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1483)",
    "AbortRunCallback at 95 (cur 983)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1482)",
    "AbortRunCallback at 95 (cur 982)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1481)",
    "AbortRunCallback at 95 (cur 981)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1480)",
    "AbortRunCallback at 95 (cur 980)",
    "IsoRunCallback 20.000000",
    "Abort2RunCallback at 95 (cur 1479)",
    "AbortRunCallback at 95 (cur 979)",
    "IsoRunCallback 20.000000",
    "EndCallback",
};


bool 
UnitTestAbort2FalseFalse(ENGINE_HANDLE Engine)
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
    Time.TimeInSeconds = 20;
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
    UtilFreeSegmentMessageBuffers( SegmentArray,
                            NumSegments,
                            Buffer);

    if (!Handle){
        printf("Failed SeCreateSegmentList(), Status = %d, BadSegmentIndex = %d\n",
                Status, SegmentIndex);
        return false;
    }
    
    UnitTestSetAbortCountDown(1000);
    UnitTestSetAbort2CountDown(1500);

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
