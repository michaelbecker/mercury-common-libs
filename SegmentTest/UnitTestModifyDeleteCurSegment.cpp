#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"



bool 
UnitTestModifyDeleteCurSegment(ENGINE_HANDLE Engine)
{
    //------------------------------------
    const int NumSegments = 3;
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
    Time.TimeInSeconds = 20000;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
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
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);
    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 21;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
                                                (unsigned char *)&Time, 
                                                sizeof(Time), 
                                                &SegmentLength[idx]);

    idx++;

    //--------------------------------------------------------------
    Time.TimeInSeconds = 22;
    SegmentArray[idx] = UtilCreateSegmentMessage( TestIso, 
                                                 Id++,
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
        if (SegmentIndex != -1){
            printf("Incorrect Segment Index ID'ed\n");
            return false;
        }
        else if (Status != SeDeletedCurrentSegment){
            printf("Incorrect Error return code.\n");
            return false;
        }
        else{
            //
            //  We are _still_ running, need to terminate it to continue.
            //  If we don't, the next unit test will fail.
            //
            SeStopSegmentList(Handle);
			SeDestroySegmentList(Handle);
            return true;
        }
    }
    else{
        printf("SeModifySegmentList() lost current index!\n");
        return false;
    }
}
