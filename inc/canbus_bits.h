/**
 *  @file   canbus_bits.h
 *
 *  @brief  Base bit definitions for the CANbus protocol.
 *
 *  Subversion Information
 *      - $HeadURL: svn://fortress/mercury/common/trunk/include/canbus_bits.h $
 *      - $Revision: 3613 $
 *      - $Date: 2015-05-07 11:21:22 -0400 (Thu, 07 May 2015) $
 *      - $Author: mbecker $
 *
 */
#ifndef CANBUS_BITS_H__
#define CANBUS_BITS_H__

/**
 *  Message bit positions.
 */
#define CAN_PRI_SHIFT   27
#define CAN_TYPE_SHIFT  23
#define CAN_MID_SHIFT   10
#define CAN_DID_SHIFT   5
#define CAN_SID_SHIFT   0

/**
 *  Masks for various message protocol components.
 */
#define CAN_PRI_MASK            (0x3 << CAN_PRI_SHIFT)
#define CAN_TYPE_MASK           (0xF << CAN_TYPE_SHIFT)
#define CAN_MID_MASK            (0x1FFF << CAN_MID_SHIFT)
#define ADDRESS_MASK            (0x1F)
#define CAN_DID_MASK            (ADDRESS_MASK << CAN_DID_SHIFT)
#define CAN_SID_MASK            (ADDRESS_MASK << CAN_SID_SHIFT)

/**
 *  Used in building messages.
 */
#define SET_CAN_PRI(pri_)       (((pri_) & 0x3)   << CAN_PRI_SHIFT)
#define SET_CAN_TYPE(mtype_)    (((mtype_) & 0xF) << CAN_TYPE_SHIFT)
#define SET_CAN_MID(id_)        (((id_) & 0x1FFF) << CAN_MID_SHIFT)
#define SET_CAN_DID(did_)       (((did_) & 0x1F)  << CAN_DID_SHIFT)
#define SET_CAN_SID(sid_)       (((sid_) & 0x1F)  << CAN_SID_SHIFT)

/**
 *  Used in parsing messages.
 */
#define GET_CAN_PRI(msg_)       ((msg_) & CAN_PRI_MASK)
#define GET_CAN_TYPE(msg_)      ((msg_) & CAN_TYPE_MASK)
#define GET_CAN_MID(msg_)       ((msg_) & CAN_MID_MASK)
#define GET_CAN_DID(msg_)       (((msg_) >> 5)  & ADDRESS_MASK) /** Special, DID is shifted to be equivelant to SID */
#define GET_CAN_SID(msg_)       (((msg_) >> 0)  & ADDRESS_MASK)

/**
 *  Helper macros
 */
#define CLEAR_SID(msg_)         ((msg_) & (~CAN_SID_MASK))
#define CLEAR_DID(msg_)         ((msg_) & (~CAN_DID_MASK))
#define CLEAR_PRI(msg_)         ((msg_) & (~CAN_PRI_MASK))  

/**
 *  A default accept mask for the libraries.  This only 
 *  ignores the Priority field.
 */
#define ACCEPT_MASK_SPECIFIC (0x07FFFFFF)

/**
 *  A default accept mask for the libraries.  This ignores
 *  the SID and Priority fields.
 */
#define ACCEPT_MASK_SPECIFIC_IGNORE_SID (0x07FFFFE0)

/**
 *  A default accept mask for the libraries.  This ignores
 *  the SID, DID and Priority fields.
 */
#define ACCEPT_MASK_SPECIFIC_IGNORE_SID_AND_DID (0x07FFFC00)

/**
 *  General defines
 */
#define MAX_RAW_CANBUS_DATA_SIZE    8

/**
 *  Extended Announcement defines.
 */
#define EXT_ANN_PACKET_DATA_LENGTH      6
#define EXT_ANN_PACKET_NUM_OFFSET       0
#define EXT_ANN_PACKET_TOTAL_OFFSET     1
#define EXT_ANN_PACKET_DATA_OFFSET      2
#define MAX_EXT_ANN_SIZE (EXT_ANN_PACKET_DATA_LENGTH * 255)

/**
 *  Command defines.
 */
#define COMMAND_TIMEOUT         300 /* ms */
#define MAX_COMMAND_RETRIES     3
#define ACK_NAK_DATA_LENGTH     4
#define MAX_COMMAND_SIZE        8

/**
 *  Extended Command defines.
 */
#define EXT_CMD_PACKET_DATA_LENGTH      6
#define EXT_CMD_PACKET_NUM_OFFSET       0
#define EXT_CMD_PACKET_TOTAL_OFFSET     1
#define EXT_CMD_PACKET_DATA_OFFSET      2
#define MAX_EXT_CMD_SIZE (EXT_CMD_PACKET_DATA_LENGTH * 255)
#define EXT_CMD_TIMEOUT         500     /* ms */
#define MAX_EXT_CMD_RETRIES     3

/**
 *  The "default" Priority on a message.
 */
#define CAN_DEFAULT_PRI SET_CAN_PRI(2)

/**
 *  The message types.
 */
#define CAN_TYPE_RSVD0              SET_CAN_TYPE(0)
#define CAN_TYPE_RSVD1              SET_CAN_TYPE(1)
#define CAN_TYPE_CMD_ACKNAK         SET_CAN_TYPE(2)
#define CAN_TYPE_EXT_CMD_ACKNAK     SET_CAN_TYPE(3)
#define CAN_TYPE_COMMAND            SET_CAN_TYPE(4)
#define CAN_TYPE_EXT_COMMAND        SET_CAN_TYPE(5)
#define CAN_TYPE_RSVD6              SET_CAN_TYPE(6)
#define CAN_TYPE_RSVD7              SET_CAN_TYPE(7)
#define CAN_TYPE_RSVD8              SET_CAN_TYPE(8)
#define CAN_TYPE_RSVD9              SET_CAN_TYPE(9)
#define CAN_TYPE_ANNOUNCEMENT       SET_CAN_TYPE(10)
#define CAN_TYPE_EXT_ANNOUNCEMENT   SET_CAN_TYPE(11)
#define CAN_TYPE_RSVD12             SET_CAN_TYPE(12)
#define CAN_TYPE_RSVD13             SET_CAN_TYPE(13)
#define CAN_TYPE_RSVD14             SET_CAN_TYPE(14)
#define CAN_TYPE_RSVD15             SET_CAN_TYPE(15)


/**
 *  Return status of various CANbus operations.
 *  These turn into NAK code.
 */
typedef enum CAN_STATUS_ {

    CsSuccess,
    CsUninitialized,
    CsTimeout,
    CsUnknownMessage,
    CsInvalidParameter,
    CsBusOff,
    CsDisconnected,
    CsBufferTooSmall,
    CsFailed,

    //
    //  Nak code above 256 are reserved for application 
    //  specific NAK codes.
    //
    CsUserNak = 0x100

}CAN_STATUS;



#endif

