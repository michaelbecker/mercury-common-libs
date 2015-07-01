#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"



bool
UnitTestInfiniteRepeatUntil(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 2;
    int Id = 0x100;
    TestIsoData_t Time;
    TestRepeatUntilData_t RepeatData;

    SegmentBits_t *SegmentArray[NumSegments];
    int SegmentLength[NumSegments];
    int BufferLength;
    SeStatus Status;
    int SegmentIndex;
    //------------------------------------

    //--------------------------------------------------------------
    Time.TimeInSeconds = 10;
    SegmentArray[0] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[0]);

    //--------------------------------------------------------------
    RepeatData.Index = 1;
    RepeatData.Heatflow = 100;
    SegmentArray[1] = UtilCreateSegmentMessage( TestRepeatUntil, 
                                                 Id++,
                                                (unsigned char *)&RepeatData, 
                                                sizeof(RepeatData),
                                                &SegmentLength[1]);

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

        if (SegmentIndex != 1){
            printf("Incorrect Segment Index ID'ed\n");
            return false;
        }
        else if (Status != SeInvalid){
            printf("Incorrect Error return code.\n");
            return false;
        }
        else{
            return true;
        }
    }
    else{
        printf("SeCreateSegmentList() accepted infinite repeat!\n");
        return false;
    }
}
