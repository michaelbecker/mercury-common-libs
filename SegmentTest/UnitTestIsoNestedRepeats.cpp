#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"


static char *ExpectedTrace[] = {
    "IsoValidateCallback 10.000000 VALID",
    "IsoValidateCallback 20.000000 VALID",
    "IsoValidateCallback 30.000000 VALID",
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
    "Segment 2",
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
    "Segment 2",
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
    "ISO 30.000000",
    "Segment 4",
    "IsoStartCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "Segment 5",
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
    "Segment 2",
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
    "Segment 2",
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
    "ISO 30.000000",
    "Segment 4",
    "IsoStartCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "IsoRunCallback 30.000000",
    "Segment 5",
    "EndCallback",
};



bool 
UnitTestIsoNestedRepeats(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 5;
    int Id = 0x100;
    TestIsoData_t Time;
    CsRepeatData_t RepeatData;

    SegmentBits_t *SegmentArray[NumSegments];
    int SegmentLength[NumSegments];
    int BufferLength;
    SeStatus Status;
    int SegmentIndex;
    int rc;
    //------------------------------------

    //--------------------------------------------------------------
    Time.TimeInSeconds = 10;
    SegmentArray[0] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[0]);

    //--------------------------------------------------------------
    RepeatData.Index = 0;
    RepeatData.Count = 1;
    SegmentArray[1] = UtilCreateSegmentMessage( CsRepeat, 
                                                 Id++,
                                                (unsigned char *)&RepeatData, 
                                                sizeof(RepeatData),
                                                &SegmentLength[1]);

    //--------------------------------------------------------------
    Time.TimeInSeconds = 20;
    SegmentArray[2] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
                                                &SegmentLength[2]);

    //--------------------------------------------------------------
    Time.TimeInSeconds = 30;
    SegmentArray[3] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
                                                &SegmentLength[3]);

    //--------------------------------------------------------------
    RepeatData.Index = 0;
    RepeatData.Count = 1;
    SegmentArray[4] = UtilCreateSegmentMessage( CsRepeat, 
                                                 Id++,
                                                (unsigned char *)&RepeatData, 
                                                sizeof(RepeatData),
                                                &SegmentLength[4]);

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
