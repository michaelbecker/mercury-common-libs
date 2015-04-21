#ifndef TA_CAN_PROTOCOL_PRIVATE_H__
#define TA_CAN_PROTOCOL_PRIVATE_H__

#include <pthread.h>
#include <semaphore.h>

#include <list>
#include <vector>

using namespace std;

typedef struct _CAN_MESSAGE_RECEIVE_STRUCT {

    unsigned int ArbitrationId;
    unsigned char *DataBuffer;  // Will always be 6*255
    unsigned int ExtCurPacket;
    unsigned int ExtLastPacket;

} CAN_MESSAGE_RECEIVE_STRUCT;


extern unsigned int CanbusSourceAddress;
extern vector<CAN_MESSAGE_RX>CanRxMessages;
extern int NumCanRxMessages;
extern pthread_mutex_t CanRxMessagesLock;
extern vector<StatusChangeCallback>CanStatusChangeCallbacks;


extern int ReadThreadHandle;


#define ADD_SOURCE_ID(msg_) ((msg_ & ~(CAN_SID_MASK)) | SET_CAN_SID(CanbusSourceAddress))



typedef struct COMMAND_ACK_NAK_EXPECTED_{

    unsigned int ExpectedAckNakId;
    int ReturnCode;
    sem_t Event;

}COMMAND_ACK_NAK_EXPECTED;

extern list<COMMAND_ACK_NAK_EXPECTED *>CmdAckNakList;
extern pthread_mutex_t CmdAckNakListLock; 

//
//  This is "the" handle to the driver.
//
extern int hCanbus;



void
SendCommandAck(unsigned int ArbitrationId);

void
SendCommandNak(unsigned int ArbitrationId, int Code);


void
SendExtCommandAck(unsigned int ArbitrationId);

void
SendExtCommandNak(unsigned int ArbitrationId, int Code);

void *
ReadThread(void *Unused);


extern PROTOCOL_STATS ProtocolStats;


#endif
