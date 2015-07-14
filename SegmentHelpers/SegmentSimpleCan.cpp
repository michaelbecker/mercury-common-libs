#include "SegmentSimpleCan.h"
#include "Version.h"


SegmentSimpleCan::SegmentSimpleCan( ENGINE_HANDLE hEngine,                      //  Which Engine you are tying this to?
                                    unsigned int SubCommand,                    //  The ACTION Subcommand.
                                    unsigned int UserFlags,                     //  Are you using user flags?
                                    unsigned int DataLength,                    //  The expected length of your data.
                                    char *Name,
                                    unsigned int segmentStartArbitrationId,     //  CANbus ID Command / Ext command to 
                                    unsigned int segmentCompleteArbitrationId,  //  CANbus ID Ann / Ext Ann where the data comes back from.
                                    unsigned int segmentModifyArbitrationId     //  CANbus ID Command / Ext command to 
                                    )
    :   ISegmentSimple(hEngine, SubCommand, UserFlags, DataLength, Name),
        SegmentComplete(false),
        InternalError(false),
        SegmentStartArbitrationId(segmentStartArbitrationId),
        SegmentCompleteArbitrationId(segmentCompleteArbitrationId),
        SegmentModifyArbitrationId(segmentModifyArbitrationId)
{
    //
    //  Can be local, the CANbus protocol library makes a copy.
    //
    CAN_MESSAGE_RX RxMessage [] = {
        {SegmentCompleteArbitrationId, ACCEPT_MASK_SPECIFIC, SegmentSimpleCan::CanbusMessageCallback, this}
    };

    CnAddMessageSlots(RxMessage, NUM_MSG_SLOTS(RxMessage));

    LogVersion();
}



SeStatus 
SegmentSimpleCan::StartCallback(unsigned char *Data)
{
    //----------------------------------------
    CAN_STATUS CanStatus;
    //----------------------------------------

    SegmentComplete = false;
    InternalError = false;

    CanStatus = CnSendMessage(  SegmentStartArbitrationId,
                                Data,
                                GetDataLength(),
                                0);

    if (CanStatus == CsSuccess){
        return SeContinue;
    }
    else{
        return SeError;
    }
}



SeStatus 
SegmentSimpleCan::RunCallback(unsigned char *Data)
{
    (void)Data;

    if (SegmentComplete){
        return SeSuccess;
    }
    else if (InternalError){
        return SeError;
    }
    else{
        return SeContinue;
    }
}



void 
SegmentSimpleCan::ModifyCallback(unsigned char *Data)
{
    //----------------------------------------
    CAN_STATUS CanStatus;
    //----------------------------------------

	//
	//	If we don't have a modify ID, just leave.
	//
	if (SegmentModifyArbitrationId == 0){
		return;
	}

    CanStatus = CnSendMessage(  SegmentModifyArbitrationId,
                                Data,
                                GetDataLength(),
                                0);

    if (CanStatus != CsSuccess){
        InternalError = true;
    }
}



CAN_STATUS  
SegmentSimpleCan::CanbusMessageCallback(    unsigned int ArbitrationId,
                                            unsigned char *Data, 
                                            int DataLength, 
                                            void *UserData)
{
    //
    //  Recover the this pointer
    //
    SegmentSimpleCan *seg = (SegmentSimpleCan *)UserData;

    seg->SegmentComplete = true;
        
    //
    //  This was a status, just return success.
    //
    return CsSuccess;
}



SegmentSimpleCan::~SegmentSimpleCan()
{
}
