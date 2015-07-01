#ifndef TEST_SEGMENT_ISO_CLASS_H__
#define TEST_SEGMENT_ISO_CLASS_H__


#include "ISegmentSimple.h"

class TestSegmentIsoClass : public ISegmentSimple {

    //
    //  Public --------------------------------------------
    //
    public:

        TestSegmentIsoClass(ENGINE_HANDLE hEngine);

        virtual SeStatus StartCallback(unsigned char *Data);

        virtual void ToStringCallback(  unsigned char *Data,
                                        char *StringBuffer,
                                        int *MaxStringBufferLength);

        virtual SeStatus ValidateCallback(unsigned char *Data);

        virtual void StopCallback(unsigned char *Data);
        virtual SeStatus RunCallback(unsigned char *Data);
        virtual void ModifyCallback(unsigned char *Data);


        void SetIsoVerbose(bool _verbose)
        {
            Verbose = _verbose;
        }

    //
    //  Private -------------------------------------------
    //
    private:

        float TestIsoIterate;
        bool Verbose;
};


#endif
