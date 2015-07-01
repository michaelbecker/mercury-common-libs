#ifndef SEGMENT_SIMPLE_CAN_H__
#define SEGMENT_SIMPLE_CAN_H__

#include "TaCanProtocol.h"
#include "ISegmentSimple.h"



class SegmentSimpleCan : public ISegmentSimple {

    //
    //  ---------------------------------------------------------------------
    //
    public:
        //  
        //  Constructor
        //
        SegmentSimpleCan(   ENGINE_HANDLE hEngine,                      //  Which Engine you are tying this to?
                            unsigned int SubCommand,                    //  The ACTION Subcommand.
                            unsigned int UserFlags,                     //  Are you using user flags?
                            unsigned int DataLength,                    //  The expected length of your data.
                            char *Name,
                            unsigned int segmentStartArbitrationId,     //  CANbus ID Command / Ext command to 
                            unsigned int segmentCompleteArbitrationId,  //  CANbus ID Ann / Ext Ann where the data comes back from.
                            unsigned int segmentModifyArbitrationId     //  CANbus ID Command / Ext command to send modified data 
																		//	(OPTIONAL, if you pass in 0 this will not be called.)
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

        virtual SeStatus RunCallback(unsigned char *Data);
        
        virtual void ModifyCallback(unsigned char *Data);

        virtual ~SegmentSimpleCan();

    //
    //  ---------------------------------------------------------------------
    //
    private:

        bool SegmentComplete;
        bool InternalError;
        unsigned int SegmentStartArbitrationId;
        unsigned int SegmentCompleteArbitrationId;
        unsigned int SegmentModifyArbitrationId;

        static CAN_STATUS  
        CanbusMessageCallback(  unsigned int ArbitrationId,
                                unsigned char *Data, 
                                int DataLength, 
                                void *UserData);


};

#endif

