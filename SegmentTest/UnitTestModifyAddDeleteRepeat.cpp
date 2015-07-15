#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"


static char *ExpectedTrace[] = {
    "IsoValidateCallback 10.000000 VALID",
    "IsoValidateCallback 10003.000000 VALID",
    "IsoValidateCallback 14.000000 VALID",
    "ISO 10.000000",
    "Segment 1",
    "IsoStartCallback 10.000000",
    "Segment 2",
    "ISO 10.000000",
    "Segment 1",
    "IsoStartCallback 10.000000",
    "Segment 2",
    "ISO 10.000000",
    "Segment 1",
    "IsoStartCallback 10.000000",
    "Segment 2",
    "ISO 10.000000",
    "Segment 1",
    "IsoStartCallback 10.000000",
    "Segment 2",
    "ISO 10003.000000",
    "Segment 3",
    "IsoStartCallback 10003.000000",
    "IsoValidateCallback 11.000000 VALID",
    "IsoValidateCallback 13.000000 VALID",
    "IsoValidateCallback 14.000000 VALID",
    "ISO 13.000000",
    "Segment 2",
    "IsoModifyCallback (New Time 13.000000)",
    "ISO 14.000000",
    "Segment 3",
    "IsoStartCallback 14.000000",
    "Segment 4",
    "ISO 13.000000",
    "Segment 2",
    "IsoStartCallback 13.000000",
    "ISO 14.000000",
    "Segment 3",
    "IsoStartCallback 14.000000",
    "Segment 4",
    "ISO 13.000000",
    "Segment 2",
    "IsoStartCallback 13.000000",
    "ISO 14.000000",
    "Segment 3",
    "IsoStartCallback 14.000000",
    "Segment 4",
    "ISO 13.000000",
    "Segment 2",
    "IsoStartCallback 13.000000",
    "ISO 14.000000",
    "Segment 3",
    "IsoStartCallback 14.000000",
    "Segment 4",
    "EndCallback",
};



bool 
UnitTestModifyAddDeleteRepeat(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 10;
//    int Id = 0x100;
    TestIsoData_t Time;
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
                                                 0x100,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    RepeatData.Index = 0;
    RepeatData.Count = 3;
    SegmentArray[idx] = UtilCreateSegmentMessage( CsRepeat, 
                                                 0x200,
                                                (unsigned char *)&RepeatData, 
                                                sizeof(RepeatData), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 10003;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x300,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;


    //--------------------------------------------------------------
    Time.TimeInSeconds = 14;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x400,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------


    unsigned char *Buffer = UtilCreateSegmentMessageList(   SegmentArray,
                                                            SegmentLength,
                                                            idx,                  
                                                            &BufferLength);

    SE_SEGMENT_LIST_HANDLE Handle = SeCreateSegmentList(Engine, 
                                                        Buffer, 
                                                        BufferLength, 
                                                        &Status, 
                                                        &SegmentIndex);
    UtilFreeSegmentMessageBuffers( SegmentArray,
                            idx,
                            Buffer);

    if (!Handle){
        printf("Failed SeCreateSegmentList(), Status = %d, BadSegmentIndex = %d\n",
                Status, SegmentIndex);
        return false;
    }

    
    SetIsoVerbose(false);

    rc = SeStartSegmentList(Handle);
    if (rc != SeSuccess){
        return false;
    }

    sleep(9);


    //--------------------------------------------------------------
    idx = 0;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 11;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x100,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 13;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x300,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;


    //--------------------------------------------------------------
    Time.TimeInSeconds = 14;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x400,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    RepeatData.Index = 1;
    RepeatData.Count = 3;
    SegmentArray[idx] = UtilCreateSegmentMessage( CsRepeat, 
                                                 0x501,
                                                (unsigned char *)&RepeatData, 
                                                sizeof(RepeatData), 
                                                &SegmentLength[idx]);

    idx++;


    //--------------------------------------------------------------
    //--------------------------------------------------------------
    Buffer = UtilCreateSegmentMessageList(  SegmentArray,
                                            SegmentLength,
                                            idx,
                                            &BufferLength);


    SE_SEGMENT_LIST_HANDLE ModifiedHandle = SeModifySegmentList(Handle, // Original handle
                                                                Buffer,
                                                                BufferLength,
                                                                &Status, 
                                                                &SegmentIndex);

    UtilFreeSegmentMessageBuffers( SegmentArray,
                            idx,
                            Buffer);

    if (!ModifiedHandle){
        printf("Failed SeModifySegmentList(), Status = %d, BadSegmentIndex = %d\n",
                Status, SegmentIndex);
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
