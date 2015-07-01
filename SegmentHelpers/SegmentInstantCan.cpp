#include "SegmentInstantCan.h"
#include "Version.h"


SegmentInstantCan::SegmentInstantCan(   ENGINE_HANDLE hEngine,                      //  Which Engine you are tying this to?
                                        unsigned int SubCommand,                    //  The ACTION Subcommand.
                                        unsigned int UserFlags,                     //  Are you using user flags?
                                        unsigned int DataLength,                    //  The expected length of your data.
                                        char *Name,
                                        unsigned int segmentStartArbitrationId     //  CANbus ID Command / Ext command to 
                                        )
    :   ISegmentInstant(hEngine, SubCommand, UserFlags, DataLength, Name),
        SegmentStartArbitrationId(segmentStartArbitrationId)
{
    LogVersion();
}



SeStatus 
SegmentInstantCan::StartCallback(unsigned char *Data)
{
    //----------------------------------------
    CAN_STATUS CanStatus;
    //----------------------------------------

    CanStatus = CnSendMessage(  SegmentStartArbitrationId,
                                Data,
                                GetDataLength(),
                                0);

    if (CanStatus == CsSuccess){
        return SeSuccess;
    }
    else{
        return SeError;
    }
}



SegmentInstantCan::~SegmentInstantCan()
{
}
