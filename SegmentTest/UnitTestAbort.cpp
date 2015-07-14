#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"


static char *ExpectedTrace[] = {
    "AbortValidateCallback 95 VALID",
    "IsoValidateCallback 1000.000000 VALID",
    "IsoValidateCallback 20.000000 VALID",
    "AbortValidateCallback 90 VALID",
    "IsoValidateCallback 3000.000000 VALID",
    "IsoValidateCallback 15.000000 VALID",
    "AbortValidateCallback 200 VALID",
    "IsoValidateCallback 5000.000000 VALID",
    "IsoValidateCallback 27.000000 VALID",
    "AbortValidateCallback 10 VALID",
    "IsoValidateCallback 10.000000 VALID",
    "IsoValidateCallback 26.000000 VALID",
    "ABORT when Count = 95",
    "Segment 1",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 95 (cur 105)",
    "ISO 1000.000000",
    "Segment 2",
    "IsoStartCallback 1000.000000",
    "AbortRunCallback at 95 (cur 104)",
    "IsoRunCallback 1000.000000",
    "AbortRunCallback at 95 (cur 103)",
    "IsoRunCallback 1000.000000",
    "AbortRunCallback at 95 (cur 102)",
    "IsoRunCallback 1000.000000",
    "AbortRunCallback at 95 (cur 101)",
    "IsoRunCallback 1000.000000",
    "AbortRunCallback at 95 (cur 100)",
    "IsoRunCallback 1000.000000",
    "AbortRunCallback at 95 (cur 99)",
    "IsoRunCallback 1000.000000",
    "AbortRunCallback at 95 (cur 98)",
    "IsoRunCallback 1000.000000",
    "AbortRunCallback at 95 (cur 97)",
    "IsoRunCallback 1000.000000",
    "AbortRunCallback at 95 (cur 96)",
    "IsoRunCallback 1000.000000",
    "AbortRunCallback Condition TRUE - Aborting next segment (95 <= 95)",
    "IsoStopCallback 1000.000000",
    "ISO 20.000000",
    "Segment 3",
    "IsoStartCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "IsoRunCallback 20.000000",
    "ABORT when Count = 90",
    "Segment 4",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 90 (cur 95)",
    "ISO 3000.000000",
    "Segment 5",
    "IsoStartCallback 3000.000000",
    "AbortRunCallback at 90 (cur 94)",
    "IsoRunCallback 3000.000000",
    "AbortRunCallback at 90 (cur 93)",
    "IsoRunCallback 3000.000000",
    "AbortRunCallback at 90 (cur 92)",
    "IsoRunCallback 3000.000000",
    "AbortRunCallback at 90 (cur 91)",
    "IsoRunCallback 3000.000000",
    "AbortRunCallback Condition TRUE - Aborting next segment (90 <= 90)",
    "IsoStopCallback 3000.000000",
    "ISO 15.000000",
    "Segment 6",
    "IsoStartCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "IsoRunCallback 15.000000",
    "ABORT when Count = 200",
    "Segment 7",
    "AbortStartCallback Condition TRUE - Skipping next segment (90 <= 200)",
    "ISO 27.000000",
    "Segment 9",
    "IsoStartCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "IsoRunCallback 27.000000",
    "ABORT when Count = 10",
    "Segment 10",
    "AbortStartCallback Condition FALSE - Attaching to next segment - abort at 10 (cur 90)",
    "ISO 10.000000",
    "Segment 11",
    "IsoStartCallback 10.000000",
    "AbortRunCallback at 10 (cur 89)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 88)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 87)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 86)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 85)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 84)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 83)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 82)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 81)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 80)",
    "IsoRunCallback 10.000000",
    "AbortRunCallback at 10 (cur 79)",
    "IsoRunCallback 10.000000",
    "ISO 26.000000",
    "Segment 12",
    "IsoStartCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "IsoRunCallback 26.000000",
    "EndCallback",
};



bool 
UnitTestAbort(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 12;
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
    Time.TimeInSeconds = 1000;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 20;
    SegmentArray[idx] = UtilCreateSegmentMessage(  TestIso,
                                                Id++, 
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    AbortData.AbortCount = 90;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestAbort, 
                                                 Id++,
                                                (unsigned char *)&AbortData, 
                                                sizeof(AbortData),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 3000;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------

    Time.TimeInSeconds = 15;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    AbortData.AbortCount = 200;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestAbort, 
                                                 Id++,
                                                (unsigned char *)&AbortData, 
                                                sizeof(AbortData),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 5000;
    SegmentArray[idx] = UtilCreateSegmentMessage(  TestIso,
                                                Id++, 
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 27;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso,  
                                                Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    AbortData.AbortCount = 10;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestAbort, 
                                                 Id++,
                                                (unsigned char *)&AbortData, 
                                                sizeof(AbortData),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 10;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 26;
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

    
    UnitTestSetAbortCountDown(105);

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
