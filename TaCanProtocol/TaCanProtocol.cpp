#include <stdio.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "TaWinAssert.h"
#include "TaCanbusApi.h"

#include "TaCanProtocol.h"
#include "canbus_bits.h"
#include "canbus_common_ids.h"
#include "TaCanProtocolPrivate.h"
#include "LogLib.h"
#include "Version.h"


vector<CAN_MESSAGE_RX>CanRxMessages;
int NumCanRxMessages = 0;
unsigned int CanbusSourceAddress;

//
//  This is safe to keep statically initialized.  There may be contention 
//  at appilcation startup, but after that when all CANbus messages have been 
//  registered, only the RxThread will be acquiring it.
//
pthread_mutex_t CanRxMessagesLock = PTHREAD_MUTEX_INITIALIZER;  // Static init ok - mb

vector<StatusChangeCallback>CanStatusChangeCallbacks;


static pthread_t ReadThreadId;

int hCanbus = -1;



void __attribute__  ((visibility ("default")))
CnGetCanbusStatistics(PCAN_STATS StatsBuffer)
{
    return;
}



static int 
AddCanStatusChangeCallback(StatusChangeCallback Callback)
{
    if (Callback == NULL){
        LogMessage("Bad StatusChangeCallback");
        return 0;
    }

    //
    //  Lock ---------------------------------------------
    //
    pthread_mutex_lock(&CanRxMessagesLock);

    CanStatusChangeCallbacks.push_back(Callback);

    //
    //  Unlock -------------------------------------------
    //
    pthread_mutex_unlock(&CanRxMessagesLock);

    return 1;
}



//
//  No way to push to the front of a vector.  So this is 
//  a simple implementation, that takes O(n) time.
//
static void
CanRxMessages_push_front(CAN_MESSAGE_RX *Message)
{
    int length = CanRxMessages.size();

    if (length != 0){

        CanRxMessages.push_back(CanRxMessages[length - 1]);

        for (unsigned int i = (length - 1); i > 0; i--){
            CanRxMessages[i] = CanRxMessages[i - 1];
        }

        CanRxMessages[0] = *Message;
    }
    else{
        CanRxMessages.push_back(*Message);
    }
}



enum AddCanMessageRxSlotsFlags {
    CmfDefault  = 0x0,
    CmfPriority = 0x1,
};



static int 
AddCanMessageRxSlots(   CAN_MESSAGE_RX *MessageRxSlots,
                        int NumMessageRxSlots,
                        unsigned int Flags
                        )
{
    //----------------------------
    int rc = 1;
    //----------------------------

    //
    //  Lock ---------------------------------------------
    //
    pthread_mutex_lock(&CanRxMessagesLock);

    //
    //  Actually make a struct copy here, use the vector to store the data.
    //  This function should only be called a few times at the start, so 
    //  optimizing the run time access is more important that optimizing here.
    //
    for (int i = 0; i < NumMessageRxSlots; i++){

        if (MessageRxSlots[i].ReceiveCallback == NULL){
            LogMessage("Bad ReceiveCallback");
            rc = 0;
            break;
        }

        if (Flags & CmfPriority){
            CanRxMessages_push_front(&MessageRxSlots[i]);
        }
        else{
            CanRxMessages.push_back(MessageRxSlots[i]);
        }

        NumCanRxMessages++;
    }

    //
    //  Unlock -------------------------------------------
    //
    pthread_mutex_unlock(&CanRxMessagesLock);

    return rc;
}



int __attribute__  ((visibility ("default"))) 
CnInitializeProtocol(   unsigned int SourceId,                  /** Who are we? */
                        CAN_MESSAGE_RX *MessageRxSlots,         /** Pointer to an Array of Messages to be read */
                        int NumMessageRxSlots,                  /** How many messages are in the Array above */
                        StatusChangeCallback StatusCallback     /** Called when things change, typically errors */
                        )
{
    //-----------------------------------------
	char *DeviceName = "/dev/ta_canbus";
    int err;
    int rc;
    pthread_mutexattr_t attr;
    //-----------------------------------------

    //
    //  Add in the Status change callback.
    //
    rc = AddCanStatusChangeCallback(StatusCallback);
    if(!rc){
        return 0;
    }

    //
    //  Save our CANbus address here, after sanity checking it.
    //
    ASSERT((SourceId & CAN_SID_MASK) == SourceId);
    ASSERT(SourceId != CAN_ID_BROADCAST);
    CanbusSourceAddress = SourceId;

    hCanbus = open(DeviceName, O_RDWR);

    if(hCanbus == -1){
		LogMessage("Failed opening CANbus!");
        return 0;
	}


    //
    //  Make sure if we exec ourselves, we close the CANbus handle.
    //
    if (fcntl(hCanbus, F_SETFD, FD_CLOEXEC) == -1) {
        LogMessage("fcntl(fd, F_SETFD, FD_CLOEXEC) Failed!");
        return 0;
    }


    //
    //  Belts and braces, make sure Self reception is disabled.
    //
    err = ioctl(hCanbus, CAN_IOCTL_DISABLE_SELF_RECEPTION, NULL);
    if (err){
        LogMessage("ioctl(CAN_IOCTL_DISABLE_SELF_RECEPTION, ...) Failed!");
        return 0;
    }


    //
    //  Enable message receipt.  We want to see messages :-)
    //
    err = ioctl(hCanbus, CAN_IOCTL_ENABLE_MESSAGE_ACCEPT, NULL);
    if (err){
        LogMessage("ioctl(CAN_IOCTL_ENABLE_MESSAGE_ACCEPT, ...) Failed!");
        return 0;
    }


    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&CmdAckNakListLock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);

    rc = pthread_create(&ReadThreadId, NULL, ReadThread, NULL);
    ASSERT(rc == 0);

    rc = AddCanMessageRxSlots(MessageRxSlots,NumMessageRxSlots, CmfDefault);

    LOG_LIBRARY_VERSION();

    return rc;
}


int __attribute__  ((visibility ("default")))
CnAddMessageSlots(  CAN_MESSAGE_RX *MessageRxSlots,     /** Pointer to an Array of Messages to be read */
                    int NumMessageRxSlots               /** How many messages are in the Array above */
                    )
{
    int rc = AddCanMessageRxSlots(MessageRxSlots,NumMessageRxSlots, CmfDefault);

    return rc;
}



int __attribute__  ((visibility ("default")))
CnAddMessageSlotsEx(    CAN_MESSAGE_RX *MessageRxSlots,     /** Pointer to an Array of Messages to be read */
                        int NumMessageRxSlots,              /** How many messages are in the Array above */
                        unsigned int Flags                  /** Flags to change list building behavior */
                        )
{
    int rc = AddCanMessageRxSlots(MessageRxSlots,NumMessageRxSlots, Flags);

    return rc;
}



int __attribute__  ((visibility ("default")))
CnAddStatusChangeCallback(  StatusChangeCallback StatusCallback     /** Called when things change, typically errors */
                            )
{
    int rc = AddCanStatusChangeCallback(StatusCallback);

    return rc;
}



/**
 *  @todo - actually implement this...
 */
void __attribute__  ((visibility ("default"))) 
CnUninitializeProtocol(void)
{
    //
    //  If we aren't closed, just get out.
    //
    if (hCanbus == -1){
        return;
    }

    pthread_mutex_destroy(&CmdAckNakListLock);

    //free(CanRxMessages);

    //  ReadThreadHandle - close this thread...
    
    close(hCanbus);
    hCanbus = -1;
}



int __attribute__  ((visibility ("default"))) 
CnResetDeviceStatistics(void)
{
    return 0;
}


