#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"


static char *ExpectedTrace[] = {
    "IsoValidateCallback 10000.000000 VALID",
    "IsoValidateCallback 17.000000 VALID",
    "ISO 10000.000000",
    "Segment 1",
    "IsoStartCallback 10000.000000",
    "IsoStopCallback 10000.000000",
    "ISO 17.000000",
    "Segment 2",
    "IsoStartCallback 17.000000",
    "EndCallback",
};


bool 
UnitTestSkipSegment(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 2;
    int Id = 0x100;
    TestIsoData_t Time;

    SegmentBits_t *SegmentArray[NumSegments];
    int SegmentLength[NumSegments];
    int BufferLength;
    SeStatus Status;
    int SegmentIndex;
    int rc;
    int idx = 0;
    //------------------------------------

    //--------------------------------------------------------------
    Time.TimeInSeconds = 10000;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 17;
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

    
    SetIsoVerbose(false);

    rc = SeStartSegmentList(Handle);
    if (rc != SeSuccess){
        return false;
    }

    sleep(2);
    SeGotoNextSegment(Handle);

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
