#ifndef SEGMENT_TEST_H__
#define SEGMENT_TEST_H__

#include <unistd.h>

#include "SegmentExecutionEngine.h"
#include "CommonSegments.h"
#include "TestSegments.h"

#include <vector>
#include <string>
using namespace std;



//
//  Helper function to build test Segments.
//
SegmentBits_t *
UtilCreateSegmentMessage(   unsigned int Id,
                            unsigned int UniqueTag,
                            unsigned char *Data,
                            int DataLength,
                            int *SegmentLength);


//
//  Helper function to build a buffer from an array of Test Segments
//
unsigned char *
UtilCreateSegmentMessageList(   SegmentBits_t **SegmentArray,
                                int SegmentLength[],
                                int NumSegments,
                                int *BufferLength);

void
UtilFreeSegmentMessageBuffers(  SegmentBits_t**SegmentArray,
                                int SegmentArrayCount,
                                unsigned char *Buffer);


void
UtilTraceExecution(char *TraceString);

void
ClearTraceExecution(void);

void
DumpTraceExecution(void);

bool
CompareTraceExecution(char *ExpectedTrace[], unsigned int ExpectedTraceSize);


//
//  Variables set from Static command callback.
//
extern bool RunEnded;
extern bool RunTerminated;
extern int CurrentSegment;

void ResetStaticCommandTestData(void);



#endif
