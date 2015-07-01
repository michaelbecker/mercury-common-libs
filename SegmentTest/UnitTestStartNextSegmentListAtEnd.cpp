#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>


#define WIN32_LEAN_AND_MEAN


#include "TaWinAssert.h"
#include "SegmentTest.h"


static char *ExpectedTrace[] = {
	"RampValidateCallback 100.000000 degC @ 10.000000 degC/min - VALID",
	"IsoValidateCallback 10.000000 VALID",
	"RampValidateCallback 201.000000 degC @ 11.000000 degC/min - VALID",
	"IsoValidateCallback 11.000000 VALID",
	"RAMP 100.000000 degC @ 10.000000 degC/min",
	"Segment 1",
	"RampStartCallback 100.000000 degC @ 10.000000 degC/min",
	"ISO 10.000000",
	"Segment 2",
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
	"EndCallback",
	"Starting ContinueHandle from EndCallback",
	"RAMP 201.000000 degC @ 11.000000 degC/min",
	"Segment 1",
	"RampStartCallback 201.000000 degC @ 11.000000 degC/min",
	"ISO 11.000000",
	"Segment 2",
	"IsoStartCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"IsoRunCallback 11.000000",
	"EndCallback",
};

extern SE_SEGMENT_LIST_HANDLE ContinueHandle;
extern SE_SEGMENT_LIST_HANDLE ContinueHandleInProgress;


bool
UnitTestStartNextSegmentListAtEnd(ENGINE_HANDLE Engine)
{
	//------------------------------------
	const int NumSegments = 2;
	int Id = 0x123;
	TestIsoData_t Time;
	TestRampData_t Ramp;

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
	SegmentArray[0] = UtilCreateSegmentMessage(	TestRamp,
												Id++,
												(unsigned char *)&Ramp,
												sizeof(Ramp),
												&SegmentLength[0]);

	//--------------------------------------------------------------
	Time.TimeInSeconds = 10;
	SegmentArray[1] = UtilCreateSegmentMessage(	TestIso,
												Id++,
												(unsigned char *)&Time,
												sizeof(Time),
												&SegmentLength[1]);


	//--------------------------------------------------------------


	unsigned char *Buffer = UtilCreateSegmentMessageList(	SegmentArray,
															SegmentLength,
															2,
															&BufferLength);

	SE_SEGMENT_LIST_HANDLE Handle = SeCreateSegmentList(	Engine,
															Buffer,
															BufferLength,
															&Status,
															&SegmentIndex);
	if (!Handle){
		printf("Failed SeCreateSegmentList(), Status = %d, BadSegmentIndex = %d\n",
			Status, SegmentIndex);
		return false;
	}

	UtilFreeSegmentMessageBuffers(	SegmentArray,
									NumSegments,
									Buffer);


	//--------------------------------------------------------------
	Ramp.TargetTempInC = 201;
	Ramp.RateInCperMin = 11;
	SegmentArray[0] = UtilCreateSegmentMessage(	TestRamp,
												Id++,
												(unsigned char *)&Ramp,
												sizeof(Ramp),
												&SegmentLength[0]);

	//--------------------------------------------------------------
	Time.TimeInSeconds = 11;
	SegmentArray[1] = UtilCreateSegmentMessage(	TestIso,
												Id++,
												(unsigned char *)&Time,
												sizeof(Time),
												&SegmentLength[1]);


	//--------------------------------------------------------------


	Buffer = UtilCreateSegmentMessageList(	SegmentArray,
											SegmentLength,
											2,
											&BufferLength);

	ContinueHandle = SeCreateSegmentList(	Engine,
											Buffer,
											BufferLength,
											&Status,
											&SegmentIndex);
	if (!ContinueHandle){
		printf("Failed SeCreateSegmentList(), Status = %d, BadSegmentIndex = %d\n",
			Status, SegmentIndex);
		return false;
	}

	UtilFreeSegmentMessageBuffers(	SegmentArray,
									NumSegments,
									Buffer);


	SetIsoVerbose(true);

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


	SeDestroySegmentList(ContinueHandleInProgress);
	SeDestroySegmentList(Handle);


	if (CompareTraceExecution(ExpectedTrace, sizeof(ExpectedTrace) / sizeof(char *))){
		return true;
	}
	else{
		return false;
	}
}
