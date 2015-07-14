#include <stdio.h>
#include <stdlib.h>

#include "TaWinAssert.h"
#include "SegmentTest.h"


static char *ExpectedTrace[] = {
    "Iso2ClassValidateCallback 11.000000 VALID",
    "Iso2ClassValidateCallback 21.000000 VALID",
    "Iso2ClassValidateCallback 31.000000 VALID",
    "ISO2CLASS 11.000000",
    "Segment 1",
    "Iso2ClassStartCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Segment 2",
    "ISO2CLASS 11.000000",
    "Segment 1",
    "Iso2ClassStartCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Segment 2",
    "ISO2CLASS 21.000000",
    "Segment 3",
    "Iso2ClassStartCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "ISO2CLASS 31.000000",
    "Segment 4",
    "Iso2ClassStartCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Segment 5",
    "ISO2CLASS 11.000000",
    "Segment 1",
    "Iso2ClassStartCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Segment 2",
    "ISO2CLASS 11.000000",
    "Segment 1",
    "Iso2ClassStartCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Iso2ClassRunCallback 11.000000",
    "Segment 2",
    "ISO2CLASS 21.000000",
    "Segment 3",
    "Iso2ClassStartCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "Iso2ClassRunCallback 21.000000",
    "ISO2CLASS 31.000000",
    "Segment 4",
    "Iso2ClassStartCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Iso2ClassRunCallback 31.000000",
    "Segment 5",
    "EndCallback",
};


bool
UnitTestIso2ClassNestedRepeats(ENGINE_HANDLE Engine)
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
    Time.TimeInSeconds = 11;
    SegmentArray[0] = UtilCreateSegmentMessage(TestIso2,
                                               Id++,
                                               (unsigned char *)&Time,
                                               sizeof(Time),
                                               &SegmentLength[0]);

    //--------------------------------------------------------------
    RepeatData.Index = 0;
    RepeatData.Count = 1;
    SegmentArray[1] = UtilCreateSegmentMessage(CsRepeat,
                                               Id++,
                                               (unsigned char *)&RepeatData,
                                               sizeof(RepeatData),
                                               &SegmentLength[1]);

    //--------------------------------------------------------------
    Time.TimeInSeconds = 21;
    SegmentArray[2] = UtilCreateSegmentMessage(TestIso2,
                                               Id++,
                                               (unsigned char *)&Time,
                                               sizeof(Time),
                                               &SegmentLength[2]);

    //--------------------------------------------------------------
    Time.TimeInSeconds = 31;
    SegmentArray[3] = UtilCreateSegmentMessage(TestIso2,
                                               Id++,
                                               (unsigned char *)&Time,
                                               sizeof(Time),
                                               &SegmentLength[3]);

    //--------------------------------------------------------------
    RepeatData.Index = 0;
    RepeatData.Count = 1;
    SegmentArray[4] = UtilCreateSegmentMessage(CsRepeat,
                                               Id++,
                                               (unsigned char *)&RepeatData,
                                               sizeof(RepeatData),
                                               &SegmentLength[4]);

    //--------------------------------------------------------------


    unsigned char *Buffer = UtilCreateSegmentMessageList(SegmentArray,
                                                         SegmentLength,
                                                         NumSegments,
                                                         &BufferLength);

    SE_SEGMENT_LIST_HANDLE Handle = SeCreateSegmentList(Engine,
                                                        Buffer,
                                                        BufferLength,
                                                        &Status,
                                                        &SegmentIndex);
    UtilFreeSegmentMessageBuffers(SegmentArray,
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
