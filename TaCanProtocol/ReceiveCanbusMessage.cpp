#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "TaWinAssert.h"
#include "TaCanbusApi.h"

#include "TaCanProtocol.h"
#include "canbus_bits.h"
#include "canbus_common_ids.h"
#include "TaCanProtocolPrivate.h"
#include "LogLib.h"
#include "Timers.h"



/**
 *  List of all "extended" messages, where we store their 
 *  required buffering as they come in.
 *  
 *  This is thread safe, because it's only ever accessed from the 
 *  reader thread.
 */
static vector<CAN_MESSAGE_RECEIVE_STRUCT *>ExtendedList;


PROTOCOL_STATS ProtocolStats;




static void
SendStatusToApplication(PCANBUS_STATUS_CHANGE StatusChange)
{
    //----------------------------------------------------
    struct timespec start;
    struct timespec end;
    struct timespec diff;
    struct timespec threshold = {0, 25 * 1000 * 1000};
    //----------------------------------------------------

    //
    //  Lock ---------------------------------------------
    //
    pthread_mutex_lock(&CanRxMessagesLock);

    for (unsigned int i = 0; i<CanStatusChangeCallbacks.size(); i++){

        now_timespec_monotonic_raw(&start);

        CanStatusChangeCallbacks[i](    StatusChange->Status1,
                                        StatusChange->Status2,
                                        StatusChange->Status3,
                                        StatusChange->Status4);

        now_timespec_monotonic_raw(&end);
        diff_timespec(&diff, &end, &start);
        int rc = compare_timespec(&diff, &threshold);
        if (rc == 1){
            LogMessage( "CanStatusChangeCallback[%d] @ 0x%p exceeded threshold! (%d, %d)", 
                        i, CanStatusChangeCallbacks[i],
                        diff.tv_sec, diff.tv_nsec);
        }
    }

    //
    //  Unlock -------------------------------------------
    //
    pthread_mutex_unlock(&CanRxMessagesLock);
}



static void 
SendProtocolErrorToApplication(int ProtocolError)
{
    //------------------------------------
    CANBUS_STATUS_CHANGE StatusChange;
    //------------------------------------

    memset(&StatusChange, 0, sizeof(CANBUS_STATUS_CHANGE));

    StatusChange.Status2 = ProtocolError;

    SendStatusToApplication(&StatusChange);
}



static CAN_STATUS 
SendMessageToApplication(   unsigned int ArbitrationId,
                            unsigned char *DataBuffer,
                            unsigned int DataBufferSize)
{
    //----------------------------------------------------
    CAN_STATUS Status = CsUnknownMessage;
    int ReceiverCount = 0;
    struct timespec start;
    struct timespec end;
    struct timespec diff;
    struct timespec threshold = {0, 25 * 1000 * 1000};
    //----------------------------------------------------

    //
    //  Lock ---------------------------------------------
    //
    pthread_mutex_lock(&CanRxMessagesLock);

    for (int i = 0; i<NumCanRxMessages; i++){

        if ((CanRxMessages[i].ArbitrationId & CanRxMessages[i].ArbitrationMask)
                == (ArbitrationId & CanRxMessages[i].ArbitrationMask)){

            now_timespec_monotonic_raw(&start);

            Status = CanRxMessages[i].ReceiveCallback(  ArbitrationId, 
                                                        DataBuffer, 
                                                        DataBufferSize, 
                                                        CanRxMessages[i].UserData);

            now_timespec_monotonic_raw(&end);
            diff_timespec(&diff, &end, &start);
            int rc = compare_timespec(&diff, &threshold);
            if (rc == 1){
                LogMessage( "ReceiveCallback(0x%x, %p, %d, %p) @ %p exceeded threshold! (%d, %d)",
                            ArbitrationId,
                            DataBuffer,
                            DataBufferSize,
                            CanRxMessages[i].UserData,
                            CanRxMessages[i].ReceiveCallback,
                            diff.tv_sec, diff.tv_nsec);
            }
            
            ReceiverCount++;
           // break;
        }
    }

    //
    //  Unlock -------------------------------------------
    //
    pthread_mutex_unlock(&CanRxMessagesLock);

    if (ReceiverCount == 0){
        ProtocolStats.ProtocolMessageNoReceivers++;
    }

    return Status;
}



static void
ReceiveExtAnnouncement(PCANBUS_MESSAGE Msg)
{
    //-------------------------------------------------
    unsigned int CurPacket;
    unsigned int LastPacket;
    unsigned int Size;
    CAN_MESSAGE_RECEIVE_STRUCT *CurExtAnn = NULL;
    unsigned int i;
    //-------------------------------------------------
    
    /**
     *  Conveience macro, describes logically what we are doing.
     */
    #define RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE() \
        {                                               \
                CurExtAnn->ExtCurPacket = 0;            \
                CurExtAnn->ExtLastPacket = 0;           \
        }                                               \


    for (i = 0; i<ExtendedList.size(); i++){
        CurExtAnn = ExtendedList[i];

        if (CurExtAnn->ArbitrationId == Msg->Id){
            break;
        }
    }

    if (i == ExtendedList.size()){
        CurExtAnn = (CAN_MESSAGE_RECEIVE_STRUCT *)malloc(sizeof (CAN_MESSAGE_RECEIVE_STRUCT));
        ASSERT(CurExtAnn != NULL);
        CurExtAnn->ArbitrationId = Msg->Id;
        CurExtAnn->DataBuffer = (unsigned char *)malloc(6 * 255);
        ASSERT(CurExtAnn->DataBuffer != NULL);
        RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE();
        ExtendedList.push_back(CurExtAnn);
    }


    //
    //  Sanity check the packet
    //
    if (Msg->DataLength < 2){
        ProtocolStats.ExtAnnBadLength++;
        LogMessage("ExtAnn: Bad Length");
        SendProtocolErrorToApplication(Csc2ExtAnnBadLength);
        RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE();
        return;
    }

    //
    //  We need this info no matter what.
    //
    CurPacket = Msg->Data[0];
    LastPacket = Msg->Data[1];

    //
    //  State 0 - we are starting to receive an Extended Announcement.
    //
    if (CurExtAnn->ExtCurPacket == 0){

        //
        //  Sanity check the protocol
        //
        if (CurPacket != 1){
            ProtocolStats.ExtAnnBadCurPacket++;
            LogMessage("ExtAnn: Bad Current Packet field!");
            SendProtocolErrorToApplication(Csc2ExtAnnBadCurPacket);
            return;
        }
        if (LastPacket == 0){
            ProtocolStats.ExtAnnBadLastPacket++;
            LogMessage("ExtAnn: Bad Last Packet Field!");
            SendProtocolErrorToApplication(Csc2ExtAnnBadLastPacket);
            return;
        }

        //
        //  If this IS the last packet (first and last).
        //  memcpy the data and make the callback.
        //
        if (CurPacket == LastPacket){

            memcpy(CurExtAnn->DataBuffer, &Msg->Data[2], Msg->DataLength - 2);
            
            (void)SendMessageToApplication( Msg->Id,
                                            CurExtAnn->DataBuffer,
                                            Msg->DataLength - 2);

            return;
        }
        //
        //  Else this is not the last packet...
        //  

        //
        //  Sanity check the data as per the protocol.
        //
        if (Msg->DataLength != 8){
            ProtocolStats.ExtAnnBadLength++;
            LogMessage("ExtAnn: Bad Length");
            SendProtocolErrorToApplication(Csc2ExtAnnBadLength);
            return;
        }

        //
        //  We are good!  Remember where we are and just wait for 
        //  additional data.
        //
        CurExtAnn->ExtCurPacket = CurPacket;
        CurExtAnn->ExtLastPacket = LastPacket;
        memcpy(CurExtAnn->DataBuffer, &Msg->Data[2], 6);

        return;
    }

    //
    //  State N - we are pulling data in.
    //
    if ((CurExtAnn->ExtCurPacket + 1) != CurExtAnn->ExtLastPacket){

        CurExtAnn->ExtCurPacket++;

        //
        //  Protocol check.
        //
        if (CurPacket != CurExtAnn->ExtCurPacket){
            ProtocolStats.ExtAnnPacketOutOfOrder++;
            LogMessage("ExtAnn: CurPacket out of order");
            SendProtocolErrorToApplication(Csc2ExtAnnPacketOutOfOrder);
            RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE();
            return;
        }
        if (LastPacket != CurExtAnn->ExtLastPacket){
            ProtocolStats.ExtAnnLastPacketChanged++;
            LogMessage("ExtAnn: Last Packet protocol failure!");
            SendProtocolErrorToApplication(Csc2ExtAnnLastPacketChanged);
            RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE();
            return;
        }

        if (Msg->DataLength != 8){
            ProtocolStats.ExtAnnBadLength++;
            LogMessage("ExtAnn: Bad Length");
            SendProtocolErrorToApplication(Csc2ExtAnnBadLength);
            RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE();
            return;
        }

        memcpy( CurExtAnn->DataBuffer + ((CurPacket - 1) * 6),
                &Msg->Data[2], 
                6);

        return;
    }


    //
    //  State N + 1 - Last data packet
    //
    if ((CurExtAnn->ExtCurPacket + 1) == CurExtAnn->ExtLastPacket){

        CurExtAnn->ExtCurPacket++;

        //
        //  Protocol check.
        //
        if (CurPacket != CurExtAnn->ExtCurPacket){
            ProtocolStats.ExtAnnPacketOutOfOrder++;
            LogMessage("ExtAnn: CurPacket out of order");
            SendProtocolErrorToApplication(Csc2ExtAnnPacketOutOfOrder);
            RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE();
            return;
        }
        if (LastPacket != CurExtAnn->ExtLastPacket){
            ProtocolStats.ExtAnnLastPacketChanged++;
            LogMessage("ExtAnn: Last Packet protocol failure");
            SendProtocolErrorToApplication(Csc2ExtAnnLastPacketChanged);
            RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE();
            return;
        }

        memcpy( CurExtAnn->DataBuffer + ((CurPacket - 1) * 6),
                &Msg->Data[2], 
                Msg->DataLength - 2);

        Size = ((CurPacket - 1) * 6) + Msg->DataLength - 2;
            
        (void)SendMessageToApplication( Msg->Id,
                                        CurExtAnn->DataBuffer,
                                        Size);

        RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE();
        return;
    }
    //
    //  Else we are out of order.  We handle this by flushing the 
    //  Extended announcement.
    //
    else{
        ProtocolStats.ExtAnnPacketOutOfOrder++;
        LogMessage("ExtAnn: Packets out of order");
        SendProtocolErrorToApplication(Csc2ExtAnnPacketOutOfOrder);
        RESET_EXTENDED_ANNOUNCEMENT_STATE_MACHINE();
        return;
    }

}



static void
ReceiveExtCommand(PCANBUS_MESSAGE Msg)
{
    //-------------------------------------------------
    unsigned int CurPacket;
    unsigned int LastPacket;
    unsigned int Size;
    CAN_MESSAGE_RECEIVE_STRUCT *CurExtCmd = NULL;
    unsigned int i;
    CAN_STATUS Status;
    //-------------------------------------------------
    
    //
    //  If the message isn't for us, we need to drop it.
    //
    if (CanbusSourceAddress != GET_CAN_DID(Msg->Id)){
        return;
    }

    /**
     *  Conveience macro, describes logically what we are doing.
     */
    #define RESET_EXTENDED_COMMAND_STATE_MACHINE()  \
        {   CurExtCmd->ExtCurPacket = 0;         \
            CurExtCmd->ExtLastPacket = 0;  }     \


    for (i = 0; i<ExtendedList.size(); i++){
        CurExtCmd = ExtendedList[i];

        if (CurExtCmd->ArbitrationId == Msg->Id){
            break;
        }
    }

    if (i == ExtendedList.size()){
        CurExtCmd = (CAN_MESSAGE_RECEIVE_STRUCT *)malloc(sizeof (CAN_MESSAGE_RECEIVE_STRUCT));
        ASSERT(CurExtCmd != NULL);
        CurExtCmd->ArbitrationId = Msg->Id;
        CurExtCmd->DataBuffer = (unsigned char *)malloc(6 * 255);
        ASSERT(CurExtCmd->DataBuffer != NULL);
        RESET_EXTENDED_COMMAND_STATE_MACHINE();
        ExtendedList.push_back(CurExtCmd);
    }


    //
    //  Sanity check the packet
    //
    if (Msg->DataLength < 2){
        ProtocolStats.ExtCmdBadLength++;
        LogMessage("ExtCmd: Bad Length");
        SendProtocolErrorToApplication(Csc2ExtCmdBadLength);
        RESET_EXTENDED_COMMAND_STATE_MACHINE();
        return;
    }

    //
    //  We need this info no matter what.
    //
    CurPacket = Msg->Data[0];
    LastPacket = Msg->Data[1];

    //
    //  State 0 - we are starting to receive an Extended Announcement.
    //
    if (CurExtCmd->ExtCurPacket == 0){

        //
        //  Sanity check the protocol
        //
        if (CurPacket != 1){
            ProtocolStats.ExtCmdBadCurPacket++;
            LogMessage("ExtCmd: Bad Current Packet field");
            SendProtocolErrorToApplication(Csc2ExtCmdBadCurPacket);
            return;
        }
        if (LastPacket == 0){
            ProtocolStats.ExtCmdBadLastPacket++;
            LogMessage("ExtCmd: Bad Last Packet Field");
            SendProtocolErrorToApplication(Csc2ExtCmdBadLastPacket);
            return;
        }

        //
        //  If this IS the last packet (first and last).
        //  memcpy the data and make the callback.
        //
        if (CurPacket == LastPacket){

            memcpy(CurExtCmd->DataBuffer, &Msg->Data[2], Msg->DataLength - 2);
            
            Status = SendMessageToApplication(  Msg->Id,
                                                CurExtCmd->DataBuffer,
                                                Msg->DataLength - 2);

            if (Status == CsSuccess)
                SendExtCommandAck(Msg->Id);
            else
                SendExtCommandNak(Msg->Id, Status);

            return;
        }
        //
        //  Else this is not the last packet...
        //  

        //
        //  Sanity check the data as per the protocol.
        //
        if (Msg->DataLength != 8){
            ProtocolStats.ExtCmdBadLength++;
            LogMessage("ExtCmd: Bad Length");
            SendProtocolErrorToApplication(Csc2ExtCmdBadLength);
            return;
        }

        //
        //  We are good!  Remember where we are and just wait for 
        //  additional data.
        //
        CurExtCmd->ExtCurPacket = CurPacket;
        CurExtCmd->ExtLastPacket = LastPacket;
        memcpy(CurExtCmd->DataBuffer, &Msg->Data[2], 6);

        return;
    }

    //
    //  State N - we are pulling data in.
    //
    if ((CurExtCmd->ExtCurPacket + 1) != CurExtCmd->ExtLastPacket){

        CurExtCmd->ExtCurPacket++;

        //
        //  Protocol check.
        //
        if (CurPacket != CurExtCmd->ExtCurPacket){
            ProtocolStats.ExtCmdPacketOutOfOrder++;
            LogMessage("ExtCmd: CurPacket out of order");
            SendProtocolErrorToApplication(Csc2ExtCmdPacketOutOfOrder);
            RESET_EXTENDED_COMMAND_STATE_MACHINE();
            return;
        }
        if (LastPacket != CurExtCmd->ExtLastPacket){
            ProtocolStats.ExtCmdLastPacketChanged++;
            SendProtocolErrorToApplication(Csc2ExtCmdLastPacketChanged);
            LogMessage("ExtCmd: Last Packet protocol failure");
            RESET_EXTENDED_COMMAND_STATE_MACHINE();
            return;
        }

        if (Msg->DataLength != 8){
            ProtocolStats.ExtCmdBadLength++;
            LogMessage("ExtAnn: Bad Length");
            SendProtocolErrorToApplication(Csc2ExtCmdBadLength);
            RESET_EXTENDED_COMMAND_STATE_MACHINE();
            return;
        }

        memcpy( CurExtCmd->DataBuffer + ((CurPacket - 1) * 6),
                &Msg->Data[2], 
                6);

        return;
    }


    //
    //  State N + 1 - Last data packet
    //
    if ((CurExtCmd->ExtCurPacket + 1) == CurExtCmd->ExtLastPacket){

        CurExtCmd->ExtCurPacket++;

        //
        //  Protocol check.
        //
        if (CurPacket != CurExtCmd->ExtCurPacket){
            ProtocolStats.ExtCmdPacketOutOfOrder++;
            LogMessage("ExtCmd: CurPacket out of order");
            SendProtocolErrorToApplication(Csc2ExtCmdPacketOutOfOrder);
            RESET_EXTENDED_COMMAND_STATE_MACHINE();
            return;
        }
        if (LastPacket != CurExtCmd->ExtLastPacket){
            ProtocolStats.ExtCmdLastPacketChanged++;
            LogMessage("ExtCmd: Last Packet protocol failure!");
            SendProtocolErrorToApplication(Csc2ExtCmdLastPacketChanged);
            RESET_EXTENDED_COMMAND_STATE_MACHINE();
            return;
        }

        memcpy( CurExtCmd->DataBuffer + ((CurPacket - 1) * 6),
                &Msg->Data[2], 
                Msg->DataLength - 2);

        Size = ((CurPacket - 1) * 6) + Msg->DataLength - 2;
            
        Status = SendMessageToApplication(  Msg->Id,
                                            CurExtCmd->DataBuffer,
                                            Size);

        if (Status == CsSuccess)
            SendExtCommandAck(Msg->Id);
        else
            SendExtCommandNak(Msg->Id, Status);

        RESET_EXTENDED_COMMAND_STATE_MACHINE();
        return;
    }
    //
    //  Else we are out of order.  We handle this by flushing the 
    //  Extended announcement.
    //
    else{
        ProtocolStats.ExtCmdPacketOutOfOrder++;
        LogMessage("ExtCmd: Packets out of order");
        SendProtocolErrorToApplication(Csc2ExtCmdPacketOutOfOrder);
        RESET_EXTENDED_COMMAND_STATE_MACHINE();
        return;
    }

}



static void
ReceiveAnnouncement(PCANBUS_MESSAGE Msg)
{
    (void)SendMessageToApplication( Msg->Id,
                                    Msg->Data,
                                    Msg->DataLength);
}



static void
ReceiveAckNak(PCANBUS_MESSAGE Msg)
{
    //--------------------------------
    int Count = 0;
    //--------------------------------

    //
    //  If the message isn't for us, we need to drop it.
    //
    if (CanbusSourceAddress != GET_CAN_DID(Msg->Id)){
        return;
    }

    if (Msg->DataLength != ACK_NAK_DATA_LENGTH){
        ProtocolStats.AckNakBadLength++;
        LogMessage("Ack/Nak: Bad Length!");
        SendProtocolErrorToApplication(Csc2AckNakBadLength);
        return;
    }

    pthread_mutex_lock(&CmdAckNakListLock);

    for (   list<COMMAND_ACK_NAK_EXPECTED *>::iterator it = CmdAckNakList.begin();
            it != CmdAckNakList.end();
            ++it){
        COMMAND_ACK_NAK_EXPECTED *AckNak = *it;
        if(AckNak->ExpectedAckNakId == Msg->Id){
            memcpy(&AckNak->ReturnCode, Msg->Data, ACK_NAK_DATA_LENGTH);
            sem_post(&AckNak->Event);
            Count++;
            break;
        }
    }

    pthread_mutex_unlock(&CmdAckNakListLock);

    if (Count == 0){
        ProtocolStats.AckNakOrphan++;
        LogMessage("Ack/Nak: Orphan!");
        SendProtocolErrorToApplication(Csc2AckNakOrphan);
    }
}



static void
ReceiveCommand(PCANBUS_MESSAGE Msg)
{
    //
    //  If the message isn't for us, we need to drop it.
    //
    if (CanbusSourceAddress != GET_CAN_DID(Msg->Id)){
        return;
    }

    CAN_STATUS Status = SendMessageToApplication(   Msg->Id,
                                                    Msg->Data,
                                                    Msg->DataLength);
    if (Status == CsSuccess)
        SendCommandAck(Msg->Id);
    else
        SendCommandNak(Msg->Id, Status);
}



static void 
ReceiveCanbusMessage(PCANBUS_MESSAGE Msg)
{
    //------------------------
    unsigned int Type;
    //------------------------

    //
    //  Clear the priority bits.
    //
    Msg->Id &= (~CAN_PRI_MASK);

    //
    //  Get the type of the Received message.
    //
    Type = GET_CAN_TYPE(Msg->Id);

    switch(Type){

        //
        //  Right now we use the same list, etc.
        //  so we want to fall through.
        //
        case CAN_TYPE_EXT_CMD_ACKNAK:
        case CAN_TYPE_CMD_ACKNAK:
            ReceiveAckNak(Msg);
            break;

        case CAN_TYPE_COMMAND:
            ReceiveCommand(Msg);
            break;

        case CAN_TYPE_EXT_COMMAND:
            ReceiveExtCommand(Msg);
            break;

        case CAN_TYPE_ANNOUNCEMENT:
            ReceiveAnnouncement(Msg);
            break;

        case CAN_TYPE_EXT_ANNOUNCEMENT:
            ReceiveExtAnnouncement(Msg);
            break;

        default:
            ProtocolStats.UnknownType++;
            LogMessage("Unknown CANbus Message type");
            SendProtocolErrorToApplication(Csc2UnknownType);
            break;
    }
}



void *ReadThread(void *Unused)
{
    //--------------------------------
    (void)Unused;
    CANBUS_MESSAGE Msg;
    int BytesRead;
    //--------------------------------
    
    //
    //  We do not need cleanup.
    //
    pthread_detach(pthread_self());

    //
    //  This should be higher priority than most of CE.  
    //  See: http://msdn.microsoft.com/en-us/library/aa450618.aspx
    //
    struct sched_param SchedulerParameters;
    int rc;
    //static int MaxFifoSchedPriority = 99;
    //static int MinFifoSchedPriority = 1;
    
    memset(&SchedulerParameters, 0, sizeof(SchedulerParameters));

    SchedulerParameters.sched_priority = 55;

    rc = sched_setscheduler (   0,
                                SCHED_FIFO,
                                &SchedulerParameters
                                );
    (void)rc;


    for(;;){

        //
        //  You MUST pass in a buffer of at least sizeof(CANBUS_MESSAGE).
        //  If you pass in larger, it will work, just be ignored.  
        //  The BytesRead may not equal sizeof(CANBUS_MESSAGE), if a 
        //  short message was received.  But it will agree with the 
        //  Datalength.  
        //  
        //  Specifically, this should always be true:
        //      BytesRead = sizeof(CANBUS_MESSAGE) - 8 + Message.DataLength
        //
        BytesRead = read(hCanbus, &Msg, sizeof(CANBUS_MESSAGE));        

        //
        //  Keep stats to monitor performance...
        //
        //QueryPerformanceCounter(&StartReceiveTime);

        ProtocolStats.ReceiveCount++;

        if (BytesRead != sizeof(CANBUS_MESSAGE)){
            LogMessage("Failed ReadFile(CAN1:, ...), aborting ReadThread!");
            return NULL;
        }


        //
        //  Check for actual message, or status change
        //
        if (Msg.Id == CANBUS_STATUS_CHANGE_FLAG){
            PCANBUS_STATUS_CHANGE StatusChange = (PCANBUS_STATUS_CHANGE)&Msg;
            SendStatusToApplication(StatusChange);
        }
        //
        //  Else handle it as usual.
        //
        else{
            ReceiveCanbusMessage(&Msg);
        }

        //QueryPerformanceCounter(&EndReceiveTime);

#if 0
        double Interval = (double)(EndReceiveTime.QuadPart - StartReceiveTime.QuadPart) / (double)PerformanceFreq.QuadPart;

        ProtocolStats.CurReceiveTime = Interval;

        if (Interval > ProtocolStats.MaxReceiveTime){
            ProtocolStats.MaxReceiveTime = Interval;
        }

        ProtocolStats.TotalReceiveTime += Interval;
        ProtocolStats.AvgReceiveTime = ProtocolStats.TotalReceiveTime / (double)ProtocolStats.ReceiveCount;
#endif        
    }
}


