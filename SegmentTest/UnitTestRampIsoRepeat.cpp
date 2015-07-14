#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"


static char *ExpectedTrace[] = {
    "RampValidateCallback 100.000000 degC @ 10.000000 degC/min - VALID",
    "IsoValidateCallback 15.000000 VALID",
    "RampValidateCallback 200.000000 degC @ 15.000000 degC/min - VALID",
    "IsoValidateCallback 5.000000 VALID",
    "RAMP 100.000000 degC @ 10.000000 degC/min",
    "Segment 1",
    "RampStartCallback 100.000000 degC @ 10.000000 degC/min",
    "ISO 15.000000",
    "Segment 2",
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
    "RAMP 200.000000 degC @ 15.000000 degC/min",
    "Segment 3",
    "RampStartCallback 200.000000 degC @ 15.000000 degC/min",
    "Segment 4",
    "RAMP 100.000000 degC @ 10.000000 degC/min",
    "Segment 1",
    "RampStartCallback 100.000000 degC @ 10.000000 degC/min",
    "ISO 15.000000",
    "Segment 2",
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
    "RAMP 200.000000 degC @ 15.000000 degC/min",
    "Segment 3",
    "RampStartCallback 200.000000 degC @ 15.000000 degC/min",
    "Segment 4",
    "RAMP 100.000000 degC @ 10.000000 degC/min",
    "Segment 1",
    "RampStartCallback 100.000000 degC @ 10.000000 degC/min",
    "ISO 15.000000",
    "Segment 2",
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
    "RAMP 200.000000 degC @ 15.000000 degC/min",
    "Segment 3",
    "RampStartCallback 200.000000 degC @ 15.000000 degC/min",
    "Segment 4",
    "RAMP 100.000000 degC @ 10.000000 degC/min",
    "Segment 1",
    "RampStartCallback 100.000000 degC @ 10.000000 degC/min",
    "ISO 15.000000",
    "Segment 2",
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
    "RAMP 200.000000 degC @ 15.000000 degC/min",
    "Segment 3",
    "RampStartCallback 200.000000 degC @ 15.000000 degC/min",
    "Segment 4",
    "ISO 5.000000",
    "Segment 5",
    "IsoStartCallback 5.000000",
    "IsoRunCallback 5.000000",
    "IsoRunCallback 5.000000",
    "IsoRunCallback 5.000000",
    "IsoRunCallback 5.000000",
    "IsoRunCallback 5.000000",
    "IsoRunCallback 5.000000",
    "EndCallback",
};




bool
UnitTestRampIsoRepeat(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 5;
    int Id = 0x123;
    TestIsoData_t Time;
    TestRampData_t Ramp;
    CsRepeatData_t RepeatData;

    SegmentBits_t *SegmentArray[NumSegments];
    int SegmentLength[NumSegments];
    int BufferLength;
    SeStatus Status;
    int SegmentIndex;
    int rc;
    //------------------------------------

    //--------------------------------------------------------------
    Ramp.TargetTempInC = 100;
    Ramp.RateInCperMin = 10;
    SegmentArray[0] = UtilCreateSegmentMessage( TestRamp, 
                                                 Id++,
                                                (unsigned char *)&Ramp, 
                                                sizeof(Ramp), 
                                                &SegmentLength[0]);

    //--------------------------------------------------------------
    Time.TimeInSeconds = 15;
    SegmentArray[1] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
                                                &SegmentLength[1]);

    //--------------------------------------------------------------
    Ramp.TargetTempInC = 200;
    Ramp.RateInCperMin = 15;
    SegmentArray[2] = UtilCreateSegmentMessage( TestRamp, 
                                                 Id++,
                                                (unsigned char *)&Ramp, 
                                                sizeof(Ramp),
                                                &SegmentLength[2]);

    //--------------------------------------------------------------
    RepeatData.Index = 0;
    RepeatData.Count = 3;
    SegmentArray[3] = UtilCreateSegmentMessage( CsRepeat, 
                                                 Id++,
                                                (unsigned char *)&RepeatData, 
                                                sizeof(RepeatData),
                                                &SegmentLength[3]);

    //--------------------------------------------------------------
    Time.TimeInSeconds = 5;
    SegmentArray[4] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time),
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
