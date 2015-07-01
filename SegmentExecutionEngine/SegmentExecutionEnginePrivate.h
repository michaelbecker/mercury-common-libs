#ifndef SEGMENT_EXECUTION_ENGINE_PRIVATE_H__
#define SEGMENT_EXECUTION_ENGINE_PRIVATE_H__

#include <pthread.h>
#include <semaphore.h>

#include <vector>
#include <map>
#include "MessageQueue.h"


using namespace std;

#define SEGMENT_SIGNATURE  0x20474553


typedef struct Segment_t_ {

    unsigned int Signature;         /**< Sanity check */
    void *Extension;                /**< Extra data, per segment */
    SegmentCommand_t *Command;      /**< Pointer to the command in the table */
    struct Segment_t_ *Conditional; /**< Conditional segment, to support ABORT segments */


    unsigned int BitsLength;
    //
    //  Copy of the actual Segment data.
    //  Must be at end of structure because this is a variable length structure.
    //
    SegmentBits_t   Bits;

}Segment_t;


/**
 *  Internal messages sent to the Engine Thread.
 */
typedef enum EngineMessageId_t_
{
    EngMsgTimeout,
    EngMsgStart,
	EngMsgStartInternal,
	EngMsgStop,
    EngMsgStopInternal,
    EngMsgModifySegmentList,
    EngMsgGotoNextSegment,
	EngMsgTerminate,

    NumSegmentEngineMessages

}EngineMessageId_t;


//
//  Need a forward reference.
//
struct SegmentList_t_;


#define ENGINE_SIGNATURE 0x4E474E45 //ENGN

typedef struct Engine_t_ {

    unsigned int Signature;

    pthread_mutex_t Lock;
    MESSAGE_QUEUE_HANDLE SegmentThreadQueue;
    pthread_t ThreadHandle;
    int RequestedThreadPriority;

    struct {

        sem_t hWait;
        int Status;

    }Messages [NumSegmentEngineMessages];

    map<unsigned int, SegmentCommand_t*> *SegmentCommandTable;

    StaticCommands_t StaticCommandTable;

    struct SegmentList_t_ *ActiveSegmentList;   /**< The active segment list. */

}Engine_t;



#define SEGMENT_LIST_SIGNATURE  0x5453494C

typedef struct SegmentList_t_ {

    unsigned int Signature;         /**< Sanity check the structure */
    vector<Segment_t *> *Segments;  /**< List of processed segments from a client. */
    Engine_t *Engine;               /**< This was created with respect to a specific engine. */
    int CurIndex;                   /**< The active segment index. */
    int BranchIndex;                /**< Where we will be branching to */
    pthread_mutex_t InUse;          /**< Mutex that siganls if a list is being processed */

}SegmentList_t;


int 
SendSyncMessage(SegmentList_t *SegmentList,
                EngineMessageId_t Id,
                unsigned char *Data,
                int Length);


void
SendAsyncMessage(   SegmentList_t *SegmentList,
                    EngineMessageId_t Id,
                    unsigned char *Data,
                    int Length);


void
RegisterRepeatSegment(Engine_t *Engine);


#endif
