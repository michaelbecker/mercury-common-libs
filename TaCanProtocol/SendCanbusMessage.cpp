#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

#include "TaWinAssert.h"
#include "TaCanbusApi.h"

#include "TaCanProtocol.h"
#include "canbus_bits.h"
#include "canbus_common_ids.h"
#include "TaCanProtocolPrivate.h"
#include "LogLib.h"
#include "Timers.h"



static bool 
SendCanbusMessage(  unsigned int ArbitrationId,
                    unsigned char *DataBuffer,
                    unsigned int DataBufferSize)
{
    //--------------------------------
    CANBUS_MESSAGE Msg;
    int BytesWritten;
    //--------------------------------

    Msg.Id = ArbitrationId;
    Msg.DataLength = DataBufferSize;
    Msg.Type = CmtExtended;
    memcpy(Msg.Data, DataBuffer, DataBufferSize);
    
    //
    //  Writes are based on a CANBUS_MESSAGE.
    //  
    //  The Data buffer size MUST BE <= sizeof(CANBUS_MESSAGE)) 
    //  and >= (sizeof(CANBUS_MESSAGE) - 8), because these are the 
    //  only valid permutations of messages.
    //  
    //  Likewise, the Data buffer size must be at least equal to 
    //  sizeof(CANBUS_MESSAGE) - 8 + Message->DataLength.  This is 
    //  just another sanity check.
    //
    BytesWritten = write(hCanbus, &Msg, sizeof(CANBUS_MESSAGE));

    if (BytesWritten != sizeof(CANBUS_MESSAGE)){
        LogMessage("Failed WriteFile(CAN1:, ...), aborting!");
        return false;
	}
    else{
        return true;
    }
}



void
SendCommandAck(unsigned int ArbitrationId)
{    
    //--------------------------------------------------
    unsigned int Mid = ArbitrationId & CAN_MID_MASK;
    unsigned int Sid = GET_CAN_SID(ArbitrationId);
    unsigned int Did = GET_CAN_DID(ArbitrationId);
    unsigned char Data[4] = {0, 0, 0, 0};
    //--------------------------------------------------

    ArbitrationId =   CAN_DEFAULT_PRI 
                        | CAN_TYPE_CMD_ACKNAK 
                        | Mid 
                        | SET_CAN_DID(Sid)
                        | SET_CAN_SID(Did);

    SendCanbusMessage(ArbitrationId, Data, 4);

    return;
}



void
SendExtCommandAck(unsigned int ArbitrationId)
{    
    //--------------------------------------------------
    unsigned int Mid = ArbitrationId & CAN_MID_MASK;
    unsigned int Sid = GET_CAN_SID(ArbitrationId);
    unsigned int Did = GET_CAN_DID(ArbitrationId);
    unsigned char Data[4] = {0, 0, 0, 0};
    //--------------------------------------------------

    ArbitrationId =   CAN_DEFAULT_PRI 
                        | CAN_TYPE_EXT_CMD_ACKNAK 
                        | Mid 
                        | SET_CAN_DID(Sid)
                        | SET_CAN_SID(Did);

    SendCanbusMessage(ArbitrationId, Data, 4);

    return;
}



void
SendCommandNak(unsigned int ArbitrationId, int Code)
{    
    //--------------------------------------------------
    unsigned int Mid = ArbitrationId & CAN_MID_MASK;
    unsigned int Sid = GET_CAN_SID(ArbitrationId);
    unsigned int Did = GET_CAN_DID(ArbitrationId);
    unsigned char Data[4];
    //--------------------------------------------------

    //
    //  Negative indicates it came from the other node.
    //
    Code = Code * -1;

    ArbitrationId =   CAN_DEFAULT_PRI 
                        | CAN_TYPE_CMD_ACKNAK 
                        | Mid 
                        | SET_CAN_DID(Sid)
                        | SET_CAN_SID(Did);

    memcpy(Data, &Code, 4);

    SendCanbusMessage(ArbitrationId, Data, 4);

    return;
}



void
SendExtCommandNak(unsigned int ArbitrationId, int Code)
{    
    //--------------------------------------------------
    unsigned int Mid = ArbitrationId & CAN_MID_MASK;
    unsigned int Sid = GET_CAN_SID(ArbitrationId);
    unsigned int Did = GET_CAN_DID(ArbitrationId);
    unsigned char Data[4];
    //--------------------------------------------------

    //
    //  Negative indicates it came from the other node.
    //
    Code = Code * -1;

    ArbitrationId =   CAN_DEFAULT_PRI 
                        | CAN_TYPE_EXT_CMD_ACKNAK 
                        | Mid 
                        | SET_CAN_DID(Sid)
                        | SET_CAN_SID(Did);

    memcpy(Data, &Code, 4);

    SendCanbusMessage(ArbitrationId, Data, 4);

    return;
}



static CAN_STATUS 
SendAnnouncementMessage(    unsigned int ArbitrationId, 
                            unsigned char *Data, 
                            unsigned int DataLength,
                            unsigned int Flags)
{
    (void)Flags;

    if (DataLength > 8){
        LogMessage("Invalid DataLength!");
        return CsInvalidParameter;
    }
    if (DataLength > 0){
        if(Data == NULL){
            LogMessage("Invalid Data Pointer!");
            return CsInvalidParameter;
        }
    }

    ArbitrationId = ADD_SOURCE_ID(ArbitrationId);
    
    bool success = SendCanbusMessage( ArbitrationId,
                                    Data,
                                    DataLength);

    if (success)
        return CsSuccess;
    else
        return CsFailed;
}



list<COMMAND_ACK_NAK_EXPECTED *>CmdAckNakList;
pthread_mutex_t CmdAckNakListLock; 


static CAN_STATUS 
SendCommandMessage( unsigned int ArbitrationId, 
                    unsigned char *Data, 
                    unsigned int DataLength,
                    unsigned int Flags)
{
    (void)Flags;
    //----------------------------
    bool success;
    int Retry = 0;
    //----------------------------

    if (DataLength > 8){
        LogMessage("Invalid DataLength!");
        return CsInvalidParameter;
    }
    if (DataLength > 0){
        if(Data == NULL){
            LogMessage("Invalid Data Pointer!");
            return CsInvalidParameter;
        }
    }

    ArbitrationId = ADD_SOURCE_ID(ArbitrationId);
    

    int Mid = ArbitrationId & CAN_MID_MASK;
    int Sid = GET_CAN_SID(ArbitrationId);
    int Did = GET_CAN_DID(ArbitrationId);

    COMMAND_ACK_NAK_EXPECTED *AckNak 
        = (COMMAND_ACK_NAK_EXPECTED *)malloc(sizeof(COMMAND_ACK_NAK_EXPECTED));
    ASSERT(AckNak != NULL);

    AckNak->ExpectedAckNakId =  CAN_TYPE_CMD_ACKNAK 
                                | Mid 
                                | SET_CAN_DID(Sid)
                                | SET_CAN_SID(Did);


    int rc = sem_init(&AckNak->Event, 0, 0);
    ASSERT(rc == 0);

    pthread_mutex_lock(&CmdAckNakListLock);
    CmdAckNakList.push_front(AckNak);
    pthread_mutex_unlock(&CmdAckNakListLock);


    for (;;){
        success = SendCanbusMessage(    ArbitrationId,
                                        Data,
                                        DataLength);

        if (!success){
            pthread_mutex_lock(&CmdAckNakListLock);
            CmdAckNakList.remove(AckNak);
            pthread_mutex_unlock(&CmdAckNakListLock);
            sem_destroy(&AckNak->Event);
            free(AckNak);
            return CsFailed;
        }

        struct timespec SemTimeout;
        //int TimeoutInMs = COMMAND_TIMEOUT * 10;
        int TimeoutInMs = COMMAND_TIMEOUT;
        set_delay_from_now(&SemTimeout, TimeoutInMs);

        rc = sem_timedwait(&AckNak->Event, &SemTimeout);

        if (rc == 0)
            break;

        if (errno == ETIMEDOUT){
            if (++Retry >= MAX_COMMAND_RETRIES)
                break;
        }
        else{
            LogMessage("sem_timedwait() Failed!");            
        }
    }

    pthread_mutex_lock(&CmdAckNakListLock);
    CmdAckNakList.remove(AckNak);
    pthread_mutex_unlock(&CmdAckNakListLock);
    sem_destroy(&AckNak->Event);

    int ReturnCode = AckNak->ReturnCode;

    free(AckNak);

    if (Retry >= MAX_COMMAND_RETRIES){
        return CsTimeout;
    }
    else{
        return (CAN_STATUS)ReturnCode;
    }
}



static CAN_STATUS 
SendExtAnnouncementMessage( unsigned int ArbitrationId, 
                            unsigned char *Data, 
                            unsigned int DataLength,
                            unsigned int Flags)
{
    //----------------------------
    int NumPackets;
    int NumFullPackets;
    int TrailingPacket;
    unsigned char Buffer[8];
    int i;
    bool success = false;
    //----------------------------

    (void)Flags;

    if(DataLength > (6 * 255)){
        LogMessage("Invalid DataLength!");
        return CsInvalidParameter;
    }
    if (DataLength > 0){
        if(Data == NULL){
            LogMessage("Invalid Data Pointer!");
            return CsInvalidParameter;
        }
    }

    ArbitrationId = ADD_SOURCE_ID(ArbitrationId);

    NumFullPackets = DataLength / 6;
    TrailingPacket = DataLength % 6;

    NumPackets = NumFullPackets;
    if (TrailingPacket)
        NumPackets++;

    for (i = 0; i < NumFullPackets; i++){
        Buffer[0] = (unsigned char)(i + 1);
        Buffer[1] = (unsigned char)NumPackets;
        memcpy(&Buffer[2], Data, 6);

        success = SendCanbusMessage( ArbitrationId,
                                       Buffer,
                                       8);
        if (!success){
            return CsFailed;
        }

        Data += 6;
        DataLength -= 6;
    }
    
    if (TrailingPacket){
        Buffer[0] = (unsigned char)(i + 1);
        Buffer[1] = (unsigned char)NumPackets;
        memcpy(&Buffer[2], Data, TrailingPacket);
        
        success = SendCanbusMessage( ArbitrationId,
                                       Buffer,
                                       TrailingPacket + 2);
        if (!success){
            return CsFailed;
        }
    }
    else if (NumPackets == 0){
        Buffer[0] = 1;
        Buffer[1] = 1;

        success = SendCanbusMessage( ArbitrationId,
                                       Buffer,
                                       2);
        if (!success){
            return CsFailed;
        }
    }

    return CsSuccess;
}



static CAN_STATUS 
SendExtCommandMessage(  unsigned int ArbitrationId, 
                        unsigned char *Data, 
                        unsigned int DataLength,
                        unsigned int Flags)
{
    //----------------------------
    int NumPackets;
    int NumFullPackets;
    int TrailingPacket;
    unsigned char Buffer[8];
    int i;
    bool success = false;
    unsigned char *SavedData;
    unsigned int SavedDataLength;
    int Retry = 0;
    int ReturnCode = CsFailed;
    //----------------------------

    (void)Flags;

    if(DataLength > (6 * 255)){
        LogMessage("Invalid DataLength!");
        return CsInvalidParameter;
    }
    if (DataLength > 0){
        if(Data == NULL){
            LogMessage("Invalid Data Pointer!");
            return CsInvalidParameter;
        }
    }

    ArbitrationId = ADD_SOURCE_ID(ArbitrationId);


    int Mid = ArbitrationId & CAN_MID_MASK;
    int Sid = GET_CAN_SID(ArbitrationId);
    int Did = GET_CAN_DID(ArbitrationId);

    COMMAND_ACK_NAK_EXPECTED *AckNak 
        = (COMMAND_ACK_NAK_EXPECTED *)malloc(sizeof(COMMAND_ACK_NAK_EXPECTED));
    ASSERT(AckNak != NULL);

    AckNak->ExpectedAckNakId =  CAN_TYPE_EXT_CMD_ACKNAK 
                                | Mid 
                                | SET_CAN_DID(Sid)
                                | SET_CAN_SID(Did);


    int rc = sem_init(&AckNak->Event, 0, 0);
    ASSERT(rc == 0);

    pthread_mutex_lock(&CmdAckNakListLock);
    CmdAckNakList.push_front(AckNak);
    pthread_mutex_unlock(&CmdAckNakListLock);

    NumFullPackets = DataLength / 6;
    TrailingPacket = DataLength % 6;

    NumPackets = NumFullPackets;
    if (TrailingPacket)
        NumPackets++;

    SavedData = Data;
    SavedDataLength = DataLength;


    for(;;){

        for (i = 0; i < NumFullPackets; i++){
            Buffer[0] = (unsigned char)(i + 1);
            Buffer[1] = (unsigned char)NumPackets;
            memcpy(&Buffer[2], Data, 6);

            success = SendCanbusMessage( ArbitrationId,
                                            Buffer,
                                            8);
            if (!success){
                goto ERROR_EXIT_WITH_CLEANUP;
            }

            Data += 6;
            DataLength -= 6;
        }
    
        if (TrailingPacket){
            Buffer[0] = (unsigned char)(i + 1);
            Buffer[1] = (unsigned char)NumPackets;
            memcpy(&Buffer[2], Data, TrailingPacket);
        
            success = SendCanbusMessage( ArbitrationId,
                                            Buffer,
                                            TrailingPacket + 2);
            if (!success){
                goto ERROR_EXIT_WITH_CLEANUP;
            }
        }
        else if (NumPackets == 0){
            Buffer[0] = 1;
            Buffer[1] = 1;

            success = SendCanbusMessage( ArbitrationId,
                                            Buffer,
                                            2);
            if (!success){
                goto ERROR_EXIT_WITH_CLEANUP;
            }
        }

        struct timespec SemTimeout;
        //int TimeoutInMs = EXT_CMD_TIMEOUT * 10;
        int TimeoutInMs = EXT_CMD_TIMEOUT;
        set_delay_from_now(&SemTimeout, TimeoutInMs);

        rc = sem_timedwait(&AckNak->Event, &SemTimeout);

        if (rc == 0)
            break;

        if (errno == ETIMEDOUT){
            if (++Retry >= MAX_EXT_CMD_RETRIES)
                break;
        }
        else{
            LogMessage("sem_timedwait() Failed!");            
            break;
        }

        //
        //  Else we timed out, reset the Data Buffer and length
        //  for the next try.
        //
        Data = SavedData;
        DataLength = SavedDataLength;
    }


    pthread_mutex_lock(&CmdAckNakListLock);
    CmdAckNakList.remove(AckNak);
    pthread_mutex_unlock(&CmdAckNakListLock);
    sem_destroy(&AckNak->Event);

    ReturnCode = AckNak->ReturnCode;

    free(AckNak);

    if (Retry >= MAX_COMMAND_RETRIES){
        return CsTimeout;
    }
    else{
        return (CAN_STATUS)ReturnCode;
    }


ERROR_EXIT_WITH_CLEANUP:
    pthread_mutex_lock(&CmdAckNakListLock);
    CmdAckNakList.remove(AckNak);
    pthread_mutex_unlock(&CmdAckNakListLock);
    sem_destroy(&AckNak->Event);
    free(AckNak);
    return CsFailed;
}



CAN_STATUS __attribute__  ((visibility ("default")))
CnSendMessage(  unsigned int ArbitrationId, 
                unsigned char *Data, 
                unsigned int DataLength,
                unsigned int Flags)
{
    //----------------------------------------
    unsigned int Type;
    CAN_STATUS Status = CsInvalidParameter;
    //----------------------------------------

    Type = GET_CAN_TYPE(ArbitrationId);

    switch(Type){

        case CAN_TYPE_COMMAND:
            Status = SendCommandMessage(  ArbitrationId, 
                                            Data, 
                                            DataLength, 
                                            Flags);
            break;

        case CAN_TYPE_EXT_COMMAND:
            Status = SendExtCommandMessage( ArbitrationId, 
                                            Data, 
                                            DataLength, 
                                            Flags);
            break;

        case CAN_TYPE_ANNOUNCEMENT:
            Status = SendAnnouncementMessage( ArbitrationId, 
                                                Data, 
                                                DataLength, 
                                                Flags);
            break;

        case CAN_TYPE_EXT_ANNOUNCEMENT:
            Status = SendExtAnnouncementMessage(  ArbitrationId, 
                                                    Data, 
                                                    DataLength, 
                                                    Flags);
            break;

        default:
            LogMessage("Invalid CANbus message type!");
            break;
    }

    return Status;
}

