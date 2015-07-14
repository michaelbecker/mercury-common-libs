#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"



static char *ExpectedTrace[] = {
    "IsoValidateCallback 10000.000000 VALID",
    "IsoValidateCallback 20000.000000 VALID",
    "ISO 10000.000000",
    "Segment 1",
    "IsoStartCallback 10000.000000",
    "IsoValidateCallback 20.000000 VALID",
    "IsoValidateCallback 21.000000 VALID",
    "IsoValidateCallback 22.000000 VALID",
    "ISO 20.000000",
    "Segment 1",
    "IsoModifyCallback (New Time 20.000000)",
    "ISO 21.000000",
    "Segment 2",
    "IsoStartCallback 21.000000",
    "ISO 22.000000",
    "Segment 3",
    "IsoStartCallback 22.000000",
    "EndCallback",
};



bool 
UnitTestModifySimple(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 3;
//    int Id = 0x100;
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
                                                 0x100,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 20000;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x200,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;


    //--------------------------------------------------------------


    unsigned char *Buffer = UtilCreateSegmentMessageList(  SegmentArray,
                                                    SegmentLength,
                                                    2,                  // Smaller list right now...
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
                            2,
                            Buffer);

    
    SetIsoVerbose(false);

    rc = SeStartSegmentList(Handle);
    if (rc != SeSuccess){
        return false;
    }

    sleep(2);


    //--------------------------------------------------------------
    idx = 0;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 20;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x100,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 21;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x101,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 22;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x102,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Buffer = UtilCreateSegmentMessageList(  SegmentArray,
                                            SegmentLength,
                                            3,
                                            &BufferLength);


    SE_SEGMENT_LIST_HANDLE ModifiedHandle = SeModifySegmentList(Handle, // Original handle
                                                                Buffer,
                                                                BufferLength,
                                                                &Status, 
                                                                &SegmentIndex);

    UtilFreeSegmentMessageBuffers( SegmentArray,
                            3,
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
