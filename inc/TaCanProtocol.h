/**
 *  @file   TaCanProtocol.h
 *
 *  @brief  Public API for sending and receiving Mercury CANbus messages 
 *          leveraging the PEAK USB / CANbus adapter.
 *  
 *  Subversion Information
 *      - $HeadURL: svn://fortress/mercury/common/trunk/at91sam7x256/bsp/include/canbus.h $
 *      - $Revision: 100 $
 *      - $Date: 2013-06-20 16:55:56 -0400 (Thu, 20 Jun 2013) $
 *      - $Author: mbecker $
 *
 */
#ifndef TA_CAN_PROTOCOL_H__
#define TA_CAN_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#include "canbus_bits.h"

//
//  Include the Driver API here.  It includes various bit fields, 
//  data structures, and errors.
//
#include "TaCanbusApi.h"


/**
 *  How to send a Mercury CANbus message.  This routine and the library 
 *  handle implementing the protocol for you.  You will get an error if
 *  you mix and match ArbitrationIds and Data lengths incorrectly.
 *  This function will block when sending a command, in order to 
 *  guarantee receiving the ACK/NAK, or a timeout.  See the CANbus
 *  spec for more details.
 * 
 *  Flags is for future expansion.  To maintain backward compatability,
 *  just pass in a zero.
 *
 *  Return status is positive if it comes internally from the sending node.
 *  If it is negative, then it came from the targetted node (it's a NAK).
*/
CAN_STATUS 
CnSendMessage(  unsigned int ArbitrationId, 
                unsigned char *Data, 
                unsigned int DataLength,
                unsigned int Flags);


/**
 *  Generic callback function that gives you a Mercury CANbus message.
 *  This may be called at any time (which is when a message comes in).
 *  You should not do long operations within here, because the message
 *  callbacks are serially called, so if you sleep for 1 second, all
 *  other messages will be delivered to you after that sleep.
 */
typedef CAN_STATUS ( *ReceiveCanbusMessageCallback)(   unsigned int ArbitrationId,
                                                                unsigned char *Data, 
                                                                int DataLength, 
                                                                void *UserData);

/**
 *  Data structure used to register for a CANbus message.  
 *
 *  You can specify not only an Arbitration ID, but also an accept mask.
 *  Details of this behavior may be found within the Atmel or CANbus spec.
 */
typedef struct _CAN_MESSAGE_RX {

    unsigned int ArbitrationId;                     /** ArbitrationId(s) you are interested in. */
    unsigned int ArbitrationMask;                   /** Accept mask to be used against the ArbitrationId */
    ReceiveCanbusMessageCallback ReceiveCallback;   /** Function to call when a message comes in that matches. Must not be NULL. */
    void *UserData;                                 /** UserData to be passed into Receive Callback */

} CAN_MESSAGE_RX;


/**
 *  Helper macro for InitializeTaCanProtocol()
 */
#define NUM_MSG_SLOTS(msg_rx_slots_)   (sizeof(msg_rx_slots_) / sizeof(CAN_MESSAGE_RX))


/**
 *  Callback for error and state changes.
 */
typedef void ( *StatusChangeCallback)( unsigned int Status1,   // This is a bitfield, CanStatusChange1, from TaCanbusApi.h
                                                unsigned int Status2,
                                                unsigned int Status3,
                                                unsigned int Status4
                                                );


/**
 *  Init the CANbus.
 *  You must call this exactly once.
 *  If you do not use this to register messages, make sure you 
 *  pass in 0 for NumMessageRxSlots.
 */
int  
CnInitializeProtocol(   unsigned int SourceId,                  /** Who are we? */
                        CAN_MESSAGE_RX *MessageRxSlots,         /** Pointer to an Array of Messages to be read */
                        int NumMessageRxSlots,                  /** How many messages are in the Array above */
                        StatusChangeCallback StatusCallback     /** Called when things change, typically errors */
                        );


/**
 *  This function may be called to add additional CAN_MESSAGE_RX 
 *  receive message blocks.  It may be called before CnInitCanbus() or 
 *  after.  
 */
int 
CnAddMessageSlots(  CAN_MESSAGE_RX *MessageRxSlots,     /** Pointer to an Array of Messages to be read */
                    int NumMessageRxSlots               /** How many messages are in the Array above */
                    );


/**
 *  Internal use only.
 */
int 
CnAddMessageSlotsEx(    CAN_MESSAGE_RX *MessageRxSlots,     /** Pointer to an Array of Messages to be read */
                        int NumMessageRxSlots,              /** How many messages are in the Array above */
                        unsigned int Flags
                        );

/** 
 *  Add additional Status change callbacks.
 */
int 
CnAddStatusChangeCallback(  StatusChangeCallback StatusCallback     /** Called when things change, typically errors */
                            );


/**
 *  Tear down CANbus.  Untested & Unimplimented
 */
void  
CnUninitializeProtocol(void);



/**
 *  Enable / Disable logging all CANbus traffic to the log file.
 *
 *  @note   FOR DEBUGGING ONLY.
 *          USING THIS MAY MESS UP THE REAL-TIME CHARACTERISTICS OF CANBUS!
 *          If you use this, you may miss messages.
 */
void 
CnEnableLogging(int Enable);




typedef struct PROTOCOL_STATS_ {
    
    unsigned long long ReceiveCount;
    double MaxReceiveTime;
    double CurReceiveTime;
    double AvgReceiveTime;
    double TotalReceiveTime;

    unsigned int ProtocolMessageNoReceivers;

    unsigned int ExtAnnBadLength;
    unsigned int ExtAnnBadCurPacket;
    unsigned int ExtAnnBadLastPacket;
    unsigned int ExtAnnPacketOutOfOrder;
    unsigned int ExtAnnLastPacketChanged;

    unsigned int ExtCmdBadLength;
    unsigned int ExtCmdBadCurPacket;
    unsigned int ExtCmdBadLastPacket;
    unsigned int ExtCmdPacketOutOfOrder;
    unsigned int ExtCmdLastPacketChanged;

    unsigned int AckNakBadLength;
    unsigned int AckNakOrphan;

    unsigned int UnknownType;

}PROTOCOL_STATS, *PPROTOCOL_STATS;


//
//  This is a simple enum, not a bitfield.  This is because we 
//  only ever report one error at a time from the protocol driver.
//
enum  CanStatusChange2 {

    Csc2NoErrors,

    Csc2ExtAnnBadLength,
    Csc2ExtAnnBadCurPacket,
    Csc2ExtAnnBadLastPacket,
    Csc2ExtAnnPacketOutOfOrder,
    Csc2ExtAnnLastPacketChanged,

    Csc2ExtCmdBadLength,
    Csc2ExtCmdBadCurPacket,
    Csc2ExtCmdBadLastPacket,
    Csc2ExtCmdPacketOutOfOrder,
    Csc2ExtCmdLastPacketChanged,

    Csc2AckNakBadLength,
    Csc2AckNakOrphan,
    Csc2UnknownType,
};



/**
 *  Track interesting CANbus status.  Many of these are 
 *  non-resettable counters.
 */
typedef struct _CAN_STATS {

    struct can_device_stats_t DeviceStats;  // Defined in TaCanbusApi.h
    struct can_file_stats_t FileStats;      // Defined in TaCanbusApi.h
    PROTOCOL_STATS ProtocolStats;   // Defined above

}CAN_STATS, *PCAN_STATS;



void 
CnGetCanbusStatistics(PCAN_STATS StatsBuffer);


int 
CnResetDeviceStatistics(void);



#ifdef __cplusplus
}
#endif


#endif


