#ifndef SEGMENT_INSTANT_CAN_H__
#define SEGMENT_INSTANT_CAN_H__

#include "TaCanProtocol.h"
#include "ISegmentInstant.h"



class SegmentInstantCan : public ISegmentInstant {

    //
    //  ---------------------------------------------------------------------
    //
    public:
        //  
        //  Constructor
        //
        SegmentInstantCan(  ENGINE_HANDLE hEngine,                      //  Which Engine you are tying this to?
                            unsigned int SubCommand,                    //  The ACTION Subcommand.
                            unsigned int UserFlags,                     //  Are you using user flags?
                            unsigned int DataLength,                    //  The expected length of your data.
                            char *Name,
                            unsigned int segmentStartArbitrationId      //  CANbus ID Command / Ext command that performs the segment 
                       );


    //
    //  ---------------------------------------------------------------------
    //
    protected:

        virtual void ToStringCallback(  unsigned char *Data,
                                        char *StringBuffer,
                                        int *MaxStringBufferLength) = 0;

        virtual SeStatus ValidateCallback(unsigned char *Data) = 0;

        virtual SeStatus StartCallback(unsigned char *Data);

        virtual ~SegmentInstantCan();

    //
    //  ---------------------------------------------------------------------
    //
    private:

        unsigned int SegmentStartArbitrationId;
};

#endif

