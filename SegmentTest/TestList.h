/**
 *  ONLY INCLUDE THIS FILE IN MAIN.CPP!!!!
 *
 *  THIS CONTAINS THE CHANGING LIST OF UNIT TESTS.
 */



typedef bool (*TestCallback_t)(ENGINE_HANDLE Engine);


struct TestCase_t {
    const char *Command;
    const char *Name;
    TestCallback_t Test;
};




/**
 *  List of unit test functions.
 */
bool UnitTestIsoNestedRepeats(ENGINE_HANDLE Engine);
bool UnitTestRampIsoRepeat(ENGINE_HANDLE Engine);
bool UnitTestInfiniteRepeat(ENGINE_HANDLE Engine);
bool UnitTestIsoRepeatUntil(ENGINE_HANDLE Engine);
bool UnitTestInfiniteRepeatUntil(ENGINE_HANDLE Engine);
bool UnitTestAbort(ENGINE_HANDLE Engine);
bool UnitTestAbortAtEndFalse(ENGINE_HANDLE Engine);
bool UnitTestAbortAtEndTrue(ENGINE_HANDLE Engine);
bool UnitTestSimpleAbortTrue(ENGINE_HANDLE Engine);
bool UnitTestSimpleAbortFalse(ENGINE_HANDLE Engine);
bool UnitTestAbortRepeatTrue(ENGINE_HANDLE Engine);
bool UnitTestAbortRepeatFalse(ENGINE_HANDLE Engine);
bool UnitTestAbort2TrueFalse(ENGINE_HANDLE Engine);
bool UnitTestAbort2FalseTrue(ENGINE_HANDLE Engine);
bool UnitTestAbort2FalseFalse(ENGINE_HANDLE Engine);
bool UnitTestSkipSegment(ENGINE_HANDLE Engine);
bool UnitTestStop(ENGINE_HANDLE Engine);
bool UnitTestTerminate(ENGINE_HANDLE Engine);
bool UnitTestModifySimple(ENGINE_HANDLE Engine);
bool UnitTestModifyDeleteCurSegment(ENGINE_HANDLE Engine);
bool UnitTestModifyComplex(ENGINE_HANDLE Engine);
bool UnitTestModifyRepeat(ENGINE_HANDLE Engine);
bool UnitTestModifyAddDeleteRepeat(ENGINE_HANDLE Engine);
bool UnitTestStartNextSegmentListAtEnd(ENGINE_HANDLE Engine);
bool UnitTestIso2ClassNestedRepeats(ENGINE_HANDLE Engine);


/**
 *  Table of unit test functions and corresponding 
 *  command line strings.
 */
struct TestCase_t TestCases[] = 
{
    { "IsoNestedRepeats",			"Multiple ISOs and 2 Nested Repeats",			UnitTestIsoNestedRepeats},
    { "RampIsoRepeat",				"Multiple Ramps, ISOs & a Repeat",				UnitTestRampIsoRepeat},
    { "InfiniteRepeat",				"Error Check an Infinite Repeat",				UnitTestInfiniteRepeat},
    { "IsoRepeatUntil",				"ISOs and 2 RepeatUntil",						UnitTestIsoRepeatUntil},
    { "InfiniteRepeatUntil",		"Error Check an Infinite RepeatUntil",			UnitTestInfiniteRepeatUntil},
    { "Abort",						"Test Aborts, both hitting them and not",		UnitTestAbort},
    { "AbortAtEndFalse",			"Test Abort as last segment - false",			UnitTestAbortAtEndFalse},
    { "AbortAtEndTrue",				"Test Abort as last segment - true",			UnitTestAbortAtEndTrue},
    { "SimpleAbortTrue",			"Simple Abort returns true",					UnitTestSimpleAbortTrue},
    { "SimpleAbortFalse",			"Simple Abort returns false",					UnitTestSimpleAbortFalse},
    { "AbortRepeatTrue",			"Abort on a Repeat returns True",				UnitTestAbortRepeatTrue},
    { "AbortRepeatFalse",			"Abort on a Repeat returns False",				UnitTestAbortRepeatFalse},
    { "AbortAbort-tf",				"2 Aborts - (true, false)",						UnitTestAbort2TrueFalse},
    { "AbortAbort-ft",				"2 Aborts - (false, true)",						UnitTestAbort2FalseTrue},
    { "AbortAbort-ff",				"2 Aborts - (false, false)",					UnitTestAbort2FalseFalse},
    { "GotoNext",					"Skip a Segment, goto next cmd",				UnitTestSkipSegment},
    { "Stop",						"Stop an experiment mid-stream",				UnitTestStop},
    { "Terminate",					"Terminate an experiment mid-stream",			UnitTestTerminate},
    { "Modify",						"Simple Modify test",							UnitTestModifySimple},
    { "ModifyComplex",				"Complex Modify test",							UnitTestModifyComplex},
    { "ModifyDelCur",				"Modify test, deleting current seg",			UnitTestModifyDeleteCurSegment},
    { "ModifyRepeat",				"Modify test with repeats",						UnitTestModifyRepeat},
    { "ModifyAddDelRepeat",			"Modify test add / del repeats",				UnitTestModifyAddDeleteRepeat},
	{ "StartNextSegmentListAtEnd",	"Check cascading start during end callback",	UnitTestStartNextSegmentListAtEnd },
    { "Iso2NestedRepeats",          "Multiple ISOs (Class ver.) and Repeats",       UnitTestIso2ClassNestedRepeats },
   
    //
    //  Sentry element, denotes the end.
    //
    {NULL, NULL, NULL}
};


