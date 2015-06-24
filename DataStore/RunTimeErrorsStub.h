/*
 *  This is a stub.
 */
#ifndef RUN_TIME_ERRORS_STUB_H__
#define RUN_TIME_ERRORS_STUB_H__

enum StubErrors {
    MajorCommonCortex_CreateFileFailed,
    MajorCommonCortex_WriteFileFailed,
    MajorCommonCortex_ReadFileFailed
};


void 
PulseError( int ErrorNumber, 
            void *OptionalData, 
            int SizeOfOptionalData);


#endif

