#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"



static char *ExpectedTrace[] = {
    "IsoValidateCallback 11.000000 VALID",
    "IsoValidateCallback 12.000000 VALID",
    "IsoValidateCallback 13.000000 VALID",
    "IsoValidateCallback 14.000000 VALID",
    "IsoValidateCallback 10005.000000 VALID",
    "IsoValidateCallback 16.000000 VALID",
    "ISO 11.000000",
    "Segment 1",
    "IsoStartCallback 11.000000",
    "ISO 12.000000",
    "Segment 2",
    "IsoStartCallback 12.000000",
    "ISO 13.000000",
    "Segment 3",
    "IsoStartCallback 13.000000",
    "ISO 14.000000",
    "Segment 4",
    "IsoStartCallback 14.000000",
    "ISO 10005.000000",
    "Segment 5",
    "IsoStartCallback 10005.000000",
    "IsoValidateCallback 130.000000 VALID",
    "IsoValidateCallback 140.000000 VALID",
    "IsoValidateCallback 15.000000 VALID",
    "IsoValidateCallback 17.000000 VALID",
    "IsoValidateCallback 18.000000 VALID",
    "ISO 15.000000",
    "Segment 3",
    "IsoModifyCallback (New Time 15.000000)",
    "ISO 17.000000",
    "Segment 4",
    "IsoStartCallback 17.000000",
    "ISO 18.000000",
    "Segment 5",
    "IsoStartCallback 18.000000",
    "EndCallback",
};



bool 
UnitTestModifyComplex(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 10;
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
    Time.TimeInSeconds = 11;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x100,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 12;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x200,
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
    Time.TimeInSeconds = 10005;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x500,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;


    //--------------------------------------------------------------
    Time.TimeInSeconds = 16;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x600,
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

    //
    //  Need time to get to "ISO 10005"
    //
    sleep(10);


    //--------------------------------------------------------------
    idx = 0;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 130;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x301,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 140;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x401,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 15;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso,
                                                 0x500,              // KEEP THIS THE SAME!!!!
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;


    //--------------------------------------------------------------
    Time.TimeInSeconds = 17;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x601,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 18;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 0x602,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

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
	SeDestroySegmentList(ModifiedHandle);

    if (CompareTraceExecution(ExpectedTrace, sizeof(ExpectedTrace) / sizeof(char *))){
        return true;
    }
    else{
        return false;
    }
}
