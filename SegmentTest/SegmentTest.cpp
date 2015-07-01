#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "SegmentExecutionEngine.h"
#include "CommonSegments.h"

#include "SegmentTest.h"

void RegisterIsoSegment(ENGINE_HANDLE Engine);
void RegisterRampSegment(ENGINE_HANDLE Engine);
void RegisterStaticCommands(ENGINE_HANDLE Engine);
void RegisterRepeatUntilSegment(ENGINE_HANDLE Engine);
void RegisterAbortSegment(ENGINE_HANDLE Engine);
void RegisterAbort2Segment(ENGINE_HANDLE Engine);


//
//  Helper function to build test Segments.
//
SegmentBits_t *
UtilCreateSegmentMessage(   unsigned int Id,
                            unsigned int UniqueTag,
                            unsigned char *Data,
                            int DataLength,
                            int *SegmentLength)
{
    //-----------------------------
    SegmentBits_t *Segment;
    //-----------------------------

    *SegmentLength = sizeof(SegmentBits_t) - 1 + DataLength;
    Segment = (SegmentBits_t *)malloc(*SegmentLength);
    Segment->SegmentTag[0] = 'S';
    Segment->SegmentTag[1] = 'G';
    Segment->SegmentTag[2] = 'M';
    Segment->SegmentTag[3] = 'T';
    Segment->Id = Id;
    Segment->UniqueTag = UniqueTag;
    memcpy(Segment->Data, Data, DataLength);

    return Segment;
}


//
//  Helper function to build a buffer from an array of Test Segments
//
unsigned char *
UtilCreateSegmentMessageList(   SegmentBits_t **SegmentArray,
                                int SegmentLength[],
                                int NumSegments,
                                int *BufferLength)
{
    //-----------------------------
    int TotalBufferNeeded = 0;
    unsigned char *Buffer;
    int Offset = 0;
    //-----------------------------

    for (int i = 0; i<NumSegments; i++){
        TotalBufferNeeded += SegmentLength[i];
    }

    *BufferLength = TotalBufferNeeded;
    Buffer = (unsigned char *)malloc(TotalBufferNeeded);

    for (int i = 0; i<NumSegments; i++){
        memcpy(Buffer + Offset, SegmentArray[i], SegmentLength[i]);
        Offset += SegmentLength[i];
    }

    return Buffer;
}


void
UtilFreeSegmentMessageBuffers(  SegmentBits_t **SegmentArray,
                                int SegmentArrayCount,
                                unsigned char *Buffer)
{
    for (int i = 0; i<SegmentArrayCount; i++){
        free(SegmentArray[i]);
    }

    free(Buffer);
}



vector<string> UnitTestTrace;


void
UtilTraceExecution(char *TraceString)
{
    UnitTestTrace.push_back(string(TraceString));
}

void
DumpTraceExecution(void)
{
    printf("Dumping Trace\n");
    printf("-------------------------\n");
    printf("static char *ExpectedTrace[] = {\n");
    for (unsigned int i = 0; i<UnitTestTrace.size(); i++){
        printf("    \"%s\",\n", UnitTestTrace[i].c_str());
    }
    printf("};\n");
}

bool
CompareTraceExecution(char *ExpectedTrace[], unsigned int ExpectedTraceSize)
{
    if (ExpectedTraceSize !=UnitTestTrace.size()){
        return false;
    }

    for (unsigned int i = 0; i<UnitTestTrace.size(); i++){
        if (strcmp(UnitTestTrace[i].c_str(), ExpectedTrace[i]) != 0){
            return false;
        }
    }

    return true;
}

void
ClearTraceExecution(void)
{
    UnitTestTrace.clear();
}



//
//  Yes, we are playing evil games with includes here...
//
#include "TestList.h"


void PrintUnitTestTitle(TestCase_t *Test)
{
    printf("----------------------------------------------------------------\n");
    printf("Running Unit Test: %s\n", Test->Name);
    printf("----------------------------------------------------------------\n");
}


void HandlePass(void)
{
    printf("+------+\n");
    printf("| Pass |\n");
    printf("+------+\n\n");
    ResetStaticCommandTestData();
    ClearTraceExecution();
}


void HandleFail(void)
{
    printf("+------+\n");
    printf("| FAIL |\n");
    printf("+------+\n\n");
    DumpTraceExecution();
    exit(-1);
}



void ExecuteTest(char *Command, ENGINE_HANDLE Engine)
{
    int i = 0;
    bool Success;

    while (TestCases[i].Command != NULL){
        if (strcasecmp(TestCases[i].Command, Command) == 0){
            PrintUnitTestTitle(&TestCases[i]);
            Success = TestCases[i].Test(Engine);
            if (Success){
                HandlePass();
            }
            else{
                HandleFail();
            }
            return;
        }
        else{
            i++;
        }
    }

    printf("Unknown Test requested!\n");
}



void ExecuteAllTests(ENGINE_HANDLE Engine)
{
    int i = 0;
    bool Success;

    while (TestCases[i].Command != NULL){
        PrintUnitTestTitle(&TestCases[i]);
        Success = TestCases[i].Test(Engine);
        if (Success){
            HandlePass();
        }
        else{
            HandleFail();
        }
        i++;
    }
}


void PrintAllTests(void)
{
    int i = 0;

    while (TestCases[i].Command != NULL){
        printf("%s - %s\n", TestCases[i].Command, TestCases[i].Name);
        i++;
    }
}




#include "TestSegmentIsoClass.h"



int main(int argc, char* argv[])
{
    //------------------------------------
    ENGINE_HANDLE Engine;
    //------------------------------------


    if (argc > 1){
        if ((strcasecmp(argv[1], "--help") == 0) ||
            (strcasecmp(argv[1], "-h") == 0) ||
            (strcasecmp(argv[1], "/help") == 0) ||
            (strcasecmp(argv[1], "/?") == 0) ||
            (strcasecmp(argv[1], "/h") == 0)){

        PrintAllTests();
        return 0;
        }
    }


    printf("\n==================================================\n");
    printf("  Automated Unit Test of SegmentExecutionEngine\n");
    printf("==================================================\n\n");

    Engine = SeCreateSegmentEngine(55);
    
    //
    //  The constructor registers the command.
    //
    TestSegmentIsoClass Iso2Command(Engine);
    Iso2Command.SetIsoVerbose(true);

    //
    //  Personalize it
    //
    RegisterStaticCommands(Engine);
    RegisterIsoSegment(Engine);
    RegisterRampSegment(Engine);
    RegisterRepeatUntilSegment(Engine);
    RegisterAbortSegment(Engine);
    RegisterAbort2Segment(Engine);


    //
    //  Run the tests.
    //
    if (argc > 1) {
        ExecuteTest(argv[1], Engine);
    }
    else {
        ExecuteAllTests(Engine);
    }

	return 0;
}

