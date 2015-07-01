#ifndef TEST_SEGMENTS_H__
#define TEST_SEGMENTS_H__

enum TestSegments {

    TestIso             = 0x7F000001,
    TestRamp            = 0x7F000002,
    TestRepeatUntil     = 0x7F000003,
    TestAbort           = 0x7F000004,
    TestAbort2          = 0x7F000005,
    TestIso2            = 0x7F000006,
};



#pragma pack(1)
typedef struct TestIsoData_t_ {

    float TimeInSeconds;

}TestIsoData_t;
#pragma pack()

void SetIsoVerbose(bool _verbose);


#pragma pack(1)
typedef struct TestRampData_t_ {

    float TargetTempInC;
    float RateInCperMin;

}TestRampData_t;
#pragma pack()



#pragma pack(1)
typedef struct TestRepeatUntilData_t_ {

    unsigned int Index;             /**< Where to branch to */
    float Heatflow;

}TestRepeatUntilData_t;
#pragma pack()

void
UnitTestSetHeatflow(float hf);



#pragma pack(1)
typedef struct TestAbortData_t_ {

    int AbortCount;

}TestAbortData_t;
#pragma pack()

void
UnitTestSetAbortCountDown(int InitialCount);

void
UnitTestSetAbort2CountDown(int InitialCount);






#endif
