#include <stdio.h>
#include <stdlib.h>


#include "TaWinAssert.h"
#include "SegmentTest.h"


//
//  This is especially tricky, because there is timing dependencies here...
//
static char *ExpectedTrace1[] = {
    "IsoValidateCallback 21.000000 VALID",
    "IsoValidateCallback 22.000000 VALID",
    "IsoValidateCallback 14.000000 VALID",
    "ISO 21.000000",
    "Segment 1",
    "IsoStartCallback 21.000000",
    "ISO 22.000000",
    "Segment 2",
    "IsoStartCallback 22.000000",
    "Segment 3",
    "ISO 21.000000",
    "Segment 1",
    "IsoStartCallback 21.000000",
    "ISO 22.000000",
    "Segment 2",
    "IsoStartCallback 22.000000",
    "IsoValidateCallback 13.000000 VALID",
    "IsoValidateCallback 14.000000 VALID",
    "IsoValidateCallback 15.000000 VALID",
    "ISO 14.000000",
    "Segment 2",
    "IsoModifyCallback (New Time 14.000000)",
    "Segment 3",
    "ISO 13.000000",
    "Segment 1",
    "IsoStartCallback 13.000000",
    "ISO 14.000000",
    "Segment 2",
    "IsoStartCallback 14.000000",
    "Segment 3",
    "ISO 13.000000",
    "Segment 1",
    "IsoStartCallback 13.000000",
    "ISO 14.000000",
    "Segment 2",
    "IsoStartCallback 14.000000",
    "Segment 3",
    "ISO 15.000000",
    "Segment 4",
    "IsoStartCallback 15.000000",
    "EndCallback",
};


static char *ExpectedTrace2[] = {
    "IsoValidateCallback 21.000000 VALID",
    "IsoValidateCallback 22.000000 VALID",
    "IsoValidateCallback 14.000000 VALID",
    "ISO 21.000000",
    "Segment 1",
    "IsoStartCallback 21.000000",
    "ISO 22.000000",
    "Segment 2",
    "IsoStartCallback 22.000000",
    "Segment 3",
    "ISO 21.000000",
    "Segment 1",
    "IsoStartCallback 21.000000",
    "ISO 22.000000",
    "Segment 2",
    "IsoStartCallback 22.000000",
    "Segment 3",
    "ISO 21.000000",
    "Segment 1",
    "IsoStartCallback 21.000000",
    "IsoValidateCallback 13.000000 VALID",
    "IsoValidateCallback 14.000000 VALID",
    "IsoValidateCallback 15.000000 VALID",
    "ISO 13.000000",
    "Segment 1",
    "IsoModifyCallback (New Time 13.000000)",
    "ISO 14.000000",
    "Segment 2",
    "IsoStartCallback 14.000000",
    "Segment 3",
    "ISO 13.000000",
    "Segment 1",
    "IsoStartCallback 13.000000",
    "ISO 14.000000",
    "Segment 2",
    "IsoStartCallback 14.000000",
    "Segment 3",
    "ISO 15.000000",
    "Segment 4",
    "IsoStartCallback 15.000000",
    "EndCallback",
};


static char *ExpectedTrace3[] = {
    "IsoValidateCallback 21.000000 VALID",
    "IsoValidateCallback 22.000000 VALID",
    "IsoValidateCallback 14.000000 VALID",
    "ISO 21.000000",
    "Segment 1",
    "IsoStartCallback 21.000000",
    "ISO 22.000000",
    "Segment 2",
    "IsoStartCallback 22.000000",
    "Segment 3",
    "ISO 21.000000",
    "Segment 1",
    "IsoStartCallback 21.000000",
    "ISO 22.000000",
    "Segment 2",
    "IsoStartCallback 22.000000",
    "Segment 3",
    "ISO 21.000000",
    "Segment 1",
    "IsoStartCallback 21.000000",
    "ISO 22.000000",
    "Segment 2",
    "IsoStartCallback 22.000000",
    "IsoValidateCallback 13.000000 VALID",
    "IsoValidateCallback 14.000000 VALID",
    "IsoValidateCallback 15.000000 VALID",
    "ISO 14.000000",
    "Segment 2",
    "IsoModifyCallback (New Time 14.000000)",
    "Segment 3",
    "ISO 13.000000",
    "Segment 1",
    "IsoStartCallback 13.000000",
    "ISO 14.000000",
    "Segment 2",
    "IsoStartCallback 14.000000",
    "Segment 3",
    "ISO 15.000000",
    "Segment 4",
    "IsoStartCallback 15.000000",
    "EndCallback",
};


bool 
UnitTestModifyRepeat(ENGINE_HANDLE Engine)
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
    Time.TimeInSeconds = 21;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x100,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 22;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x200,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;


    //--------------------------------------------------------------
    RepeatData.Index = 0;
    RepeatData.Count = 100;
    SegmentArray[idx] = UtilCreateSegmentMessage( CsRepeat, 
                                                 0x300,
                                                (unsigned char *)&RepeatData, 
                                                sizeof(RepeatData), 
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
    if (!Handle){
        printf("Failed SeCreateSegmentList(), Status = %d, BadSegmentIndex = %d\n",
                Status, SegmentIndex);
        return false;
    }

    UtilFreeSegmentMessageBuffers( SegmentArray,
                            idx,
                            Buffer);

    
    SetIsoVerbose(false);

    rc = SeStartSegmentList(Handle);
    if (rc != SeSuccess){
        return false;
    }

    sleep(3);


    //--------------------------------------------------------------
    idx = 0;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 13;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x100,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 14;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x200,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;


    //--------------------------------------------------------------
    RepeatData.Index = 0;
    RepeatData.Count = 3;
    SegmentArray[idx] = UtilCreateSegmentMessage( CsRepeat, 
                                                 0x300,
                                                (unsigned char *)&RepeatData, 
                                                sizeof(RepeatData), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 15;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x400,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
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

    if (!ModifiedHandle){
        printf("Failed SeModifySegmentList(), Status = %d, BadSegmentIndex = %d\n",
                Status, SegmentIndex);
        return false;
    }

    UtilFreeSegmentMessageBuffers( SegmentArray,
                            idx,
                            Buffer);


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

    if (CompareTraceExecution(ExpectedTrace1, sizeof(ExpectedTrace1) / sizeof(char *))){
        printf("Matched ExpectedTrace1\n");
        return true;
    }
    else if (CompareTraceExecution(ExpectedTrace2, sizeof(ExpectedTrace2) / sizeof(char *))){
        printf("Matched ExpectedTrace2\n");
        return true;
    }
    else if (CompareTraceExecution(ExpectedTrace3, sizeof(ExpectedTrace3) / sizeof(char *))){
        printf("Matched ExpectedTrace3\n");
        return true;
    }
    else{
        return false;
    }
}
