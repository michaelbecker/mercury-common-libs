/**
 *  The bit and structure definitions for the Communications structures.
 */
#ifndef COMMON_INSTRUMENT_INTERFACE_BITS_H__
#define COMMON_INSTRUMENT_INTERFACE_BITS_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 *  The standard message types that the CII understands.
 */
typedef enum MessageType_ {

    MtUninitialized = 0x0,

    MtGetCommand = 0x20544547,      /**< "GET " */
    MtActionCommand = 0x4E544341,   /**< "ACTN" */

    MtLogin = 0x4E474F4C,           /**< "LOGN" */
    MtAccept = 0x54504341,          /**< "ACPT" */

    MtAck = 0x204B4341,             /**< "ACK " */
    MtNak = 0x204B414E,             /**< "NAK " */
    MtResponse = 0x20505352,        /**< "RSP " */
    MtStatus = 0x54415453,          /**< "STAT" */

} MessageType;


/**
 *  Return status from the message comm callbacks.
 */
typedef enum MsgStatus_ {

    MsPending = -1,             /**<  Internal status, the app will finish later */
    MsSuccess = 0,              /**<  It worked. */
    MsFailed = 1,               /**<  It didn't work. */
    MsUnknownCommand = 2,       /**<  Unknown command message. */
    MsMalformedMessage = 3,     /**<  Trouble parsing the message at the protocol level */
    MsBusy = 4,                 /**<  Try again later... */
    MsNotLoggedIn = 5,          /**<  Try again later... */
    MsAccessDenied = 6,         /**<  Try again later... */
    MsOperationTimedOut = 7,    /**<  Internal Timeout */
    MsDeprecated = 8,           /**<  Message is no longer supported. */
    MsUserSpecific = 256,       /**<  Everything past this is custom */

}MsgStatus;



/**
 *  A mapping of the various base structures that can be sent 
 *  or received on the wire.
 */ 
#pragma pack(1)
typedef struct CiiMessageData_{
        
    /**
     *  The type always sent.  Defines the rest of the data structure.
     */
    MessageType Type;

    /**
     *  Various data structures corelating to the various types.
     */
    union {

        struct {
            unsigned int SequenceNumber;
            unsigned int SubCommand;
            unsigned char Data[1];
        }Get;

        struct {
            unsigned int SequenceNumber;
            unsigned int SubCommand;
            unsigned char Data[1];
        }Action;

        struct {
            unsigned int SequenceNumber;
            unsigned int SubCommand;
            unsigned int StatusCode;
            unsigned char Data[1];
        }Rsp;

        struct {
            unsigned int SequenceNumber;
        }Ack;

        struct {
            unsigned int SequenceNumber;
            unsigned int ErrorCode;
        }Nak;

        struct {
            unsigned int SubStatus;
            unsigned char Data[1];
        }Status;

        struct {
            unsigned char Data[1];
        }Login;

        struct {
            unsigned int GrantedAccess;
        }LoginAccept;
    };

}CiiMessageData;



/**
 *  Helper macros to tear the above data structure apart into useful data.
 */
#define MEMBER_SIZEOF(type_, member_)   sizeof(((type_ *)0)->member_)
#define ACK_SIZE                        (MEMBER_SIZEOF(CiiMessageData, Type) + MEMBER_SIZEOF(CiiMessageData, Ack))
#define NAK_SIZE                        (MEMBER_SIZEOF(CiiMessageData, Type) + MEMBER_SIZEOF(CiiMessageData, Nak))
#define ACCEPT_SIZE                     (MEMBER_SIZEOF(CiiMessageData, Type) + MEMBER_SIZEOF(CiiMessageData, LoginAccept))
#define GET_HEADER_SIZE                 (MEMBER_SIZEOF(CiiMessageData, Type) + MEMBER_SIZEOF(CiiMessageData, Get) - 1)
#define ACTION_HEADER_SIZE              (MEMBER_SIZEOF(CiiMessageData, Type) + MEMBER_SIZEOF(CiiMessageData, Action) - 1)
#define RESPONSE_HEADER_SIZE            (MEMBER_SIZEOF(CiiMessageData, Type) + MEMBER_SIZEOF(CiiMessageData, Rsp) - 1)
#define STATUS_HEADER_SIZE              (MEMBER_SIZEOF(CiiMessageData, Type) + MEMBER_SIZEOF(CiiMessageData, Status) - 1)
#define LOGIN_HEADER_SIZE               (MEMBER_SIZEOF(CiiMessageData, Type) + MEMBER_SIZEOF(CiiMessageData, Login) - 1)

#define GET_DATA(msg_)                  (msg_->MessageData->Get.Data)
#define GET_DATA_LENGTH(msg_)           (msg_->MessageDataLength - GET_HEADER_SIZE)
#define ACTION_DATA(msg_)               (msg_->MessageData->Action.Data)
#define ACTION_DATA_LENGTH(msg_)        (msg_->MessageDataLength - ACTION_HEADER_SIZE)

#define STATUS_DATA(msg_)               (msg_->MessageData->Status.Data)
#define STATUS_DATA_LENGTH(msg_)        (msg_->MessageDataLength - STATUS_HEADER_SIZE)

#define LOGIN_DATA(msg_)                (msg_->MessageData->Login.Data)
#define LOGIN_DATA_LENGTH(msg_)         (msg_->MessageDataLength - LOGIN_HEADER_SIZE)


/**
 *  Access levels for the Mercury Instruments.
 */
typedef enum AccessLevel_{

    AlInvalid = 0,
    AlViewOnly = 1,
    AlMaster = 2,
    AlLocalUI = 3,
    AlEngineering = 1000

}AccessLevel;



#pragma pack(1)

typedef struct LoginInfo_t_ {

    unsigned char ClientIP[4];
    char UserName[64];
    char MachineName[64];

}LoginInfo_t;
#pragma pack()


/**
 *  Login structure from the spec.  The Cii needs to save a list of these, 
 *  so it's leveraging the ClientRegistration to do it.
 */
#pragma pack(1)
typedef struct Login_t_{

    AccessLevel RequestedAccess;    /**< What Access level do you want? */
    LoginInfo_t Info;

}Login_t;
#pragma pack()



enum TypeRangeStart {
    GetRangeStart       = 0x00000,
    ActionRangeStart    = 0x10000,
    StatusRangeStart    = 0x20000,
    
    //  Add new message types here, 
    //  From (0 - 0xFF):0000

};

enum InstrumentRangeStart {

    CommonRangeStart    = 0x0000000,
    DscRangeStart       = 0x1000000,
    TgaRangeStart       = 0x2000000,
    SdtRangeStart       = 0x3000000,
    SaRangeStart        = 0x4000000,
    DmaRangeStart       = 0x5000000,
    TmaRangeStart       = 0x6000000,

    //  Add new instruments here, 
    //  From (0 - 0x7E) 00:0000

    DebugRangeStart     = 0x7F000000
};



#pragma pack(1)

typedef struct DbgClientConnection_t_ {

    unsigned int ConnectionHandle;
    AccessLevel GrantedAccess;
    Login_t Login;

}DbgClientConnection_t;

#pragma pack()



#ifdef __cplusplus
}
#endif


#endif
