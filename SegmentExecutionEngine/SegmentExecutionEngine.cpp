#include <string.h>
#include <stdio.h>
#include "TaWinAssert.h"
#include "SegmentExecutionEngine.h"
#include "SegmentExecutionEnginePrivate.h"
#include "SegmentBits.h"
#include "CommonSegments.h"
#include "Version.h"


using namespace std;



int 
SendSyncMessage(SegmentList_t *SegmentList,
                EngineMessageId_t Id,
                unsigned char *Data,
                int Length)
{
    //-----------------------------------------
    int Status;
    MESSAGE_QUEUE_HANDLE MessageQueueHandle;
    Engine_t *Engine;
    //-----------------------------------------
    
    Engine = SegmentList->Engine;
    MessageQueueHandle = Engine->SegmentThreadQueue;

    //
    //  LOCK ---------------------------------------------
    //
    pthread_mutex_lock(&Engine->Lock);

    //
    //  Send the message to the Engine Thread.
    //
    MqQueueMessage(MessageQueueHandle, Id, Length, Data);

    //
    //  Wait for a reply.
    //
    sem_wait(&Engine->Messages[Id].hWait);

    //
    //  Get the status from the action.
    //
    Status = Engine->Messages[Id].Status;

    //
    //  UNLOCK -------------------------------------------
    //
    pthread_mutex_unlock(&Engine->Lock);

    return Status;
}



void
SendAsyncMessage(   SegmentList_t *SegmentList,
                    EngineMessageId_t Id,
                    unsigned char *Data,
                    int Length)
{
    //-----------------------------------------
    MESSAGE_QUEUE_HANDLE MessageQueueHandle;
    Engine_t *Engine;
    //-----------------------------------------
    
    Engine = SegmentList->Engine;
    MessageQueueHandle = Engine->SegmentThreadQueue;

    //
    //  Send the message to the Engine Thread.
    //
    MqQueueMessage(MessageQueueHandle, Id, Length, Data);
}



static void
CompleteSyncMessage(Engine_t *Engine, 
                    int Id, 
                    int Status)
{
    //
    //  MUST do this in this order!
    //
    Engine->Messages[Id].Status = Status;
    sem_post(&Engine->Messages[Id].hWait);
}




/**
 *  Run only after validate!
 */
void
SetupAllSegments(SegmentList_t *SegmentList)
{
    //----------------------------------------------------------------------
    Segment_t *Segment;
    SegmentCommand_t *Command;
    //----------------------------------------------------------------------

    for (unsigned int i = 0; i<SegmentList->Segments->size(); i++){

        Segment = (*SegmentList->Segments)[i];
        Command = Segment->Command;

        if (Command->Setup != NULL) {

            Segment->Extension = Command->Setup(SegmentList,
                                                Segment->Bits.Data, 
                                                Command->UserData);
        }
    }

    return;
}



/**
 *  Run only after validate!
 */
void
CleanupAllSegments(SegmentList_t *SegmentList)
{
    //----------------------------------------------------------------------
    Segment_t *Segment;
    SegmentCommand_t *Command;
    //----------------------------------------------------------------------

    for (unsigned int i = 0; i<SegmentList->Segments->size(); i++){

        Segment = (*SegmentList->Segments)[i];
        Command = Segment->Command;

        if (Command->Cleanup != NULL) {
            Command->Cleanup(   SegmentList,
                                Segment->Bits.Data, 
                                Command->UserData,
                                Segment->Extension);
        }
    }

    return;
}



static void
LogStartOfNewSegment(   Engine_t *Engine,
                        SegmentCommand_t *Command,
                        Segment_t *Segment)
{
    //-------------------------------
    char LogBuffer[256];
    int Length;
    //-------------------------------

    Length = sizeof(LogBuffer);

    Command->ToString(  Engine->ActiveSegmentList,
                        Segment->Bits.Data, 
                        Command->UserData, 
                        Segment->Extension,
                        LogBuffer, 
                        &Length
                        );

    Engine->StaticCommandTable.SegmentChanged(  Engine->ActiveSegmentList->CurIndex, 
                                                &Segment->Bits,
                                                Segment->BitsLength,
                                                LogBuffer,
                                                Engine->StaticCommandTable.SegmentChangedUserData);
}



/**
 *  Internal return status used 
 *  between the Engine and it's helper functions.
 */
enum EngineRunStatus {
    EngRunStatInvalid,
    EngRunStatContinue,
    EngRunStatEnd,
    EngRunStatTerminate
};



void
DeleteChainedConditionals(Segment_t *Segment)
{
    while (Segment->Conditional){
        Segment_t *Prior = Segment;
        Segment = Segment->Conditional;
        Prior->Conditional = NULL;
    }
}



static void
StopActiveSegment(SegmentList_t *SegmentList)
{
    //-----------------------------------------------------------------------
    SegmentCommand_t *Command;
    Segment_t *Segment;
    int TotalNumSegments;
    //-----------------------------------------------------------------------

    //
    //  If we were at the end, we were already done.
    //
    TotalNumSegments = (*SegmentList->Segments).size();

    if (SegmentList->CurIndex >= TotalNumSegments){
        return;
    }

    //
    //  Get the data needed to run the next step...
    //
    Segment = (*SegmentList->Segments)[SegmentList->CurIndex];
    Command = Segment->Command;

    //
    //  Just make sure that the conditional segments are NULL.
    //
    DeleteChainedConditionals(Segment);

    if (Command->Stop){

        Command->Stop(  SegmentList,
                        Segment->Bits.Data, 
                        Command->UserData, 
                        Segment->Extension);
    }

    return;
}


static void
ModifyActiveSegment(SegmentList_t *SegmentList)
{
    //-----------------------------------------------------------------------
    SegmentCommand_t *Command;
    Segment_t *Segment;
    int TotalNumSegments;
    //-----------------------------------------------------------------------

    //
    //  If we were at the end, we were already done.
    //
    TotalNumSegments = (*SegmentList->Segments).size();

    if (SegmentList->CurIndex >= TotalNumSegments){
        return;
    }

    //
    //  Get the data needed to run the next step...
    //
    Segment = (*SegmentList->Segments)[SegmentList->CurIndex];
    Command = Segment->Command;

    if (Command->Modify){

        LogStartOfNewSegment(SegmentList->Engine, Command, Segment);

        Command->Modify(SegmentList,
                        Segment->Bits.Data, 
                        Command->UserData, 
                        Segment->Extension);
    }

    return;
}




static EngineRunStatus
SkipSegment(SegmentList_t *SegmentList)
{
    //------------------------
    int TotalNumSegments;
    //------------------------

    StopActiveSegment(SegmentList);

    //
    //  If we were at the end, we were already done.
    //
    TotalNumSegments = (*SegmentList->Segments).size();

    if (SegmentList->CurIndex >= TotalNumSegments){
        return EngRunStatEnd;
    }

    SegmentList->CurIndex++;

    if (SegmentList->CurIndex >= TotalNumSegments){
        return EngRunStatEnd;
    }
    else{
        return EngRunStatContinue;
    }
}



static SeStatus 
StartSegmentWithLocks(  SegmentList_t *SegmentList,
                        SegmentCommand_t *Command,
                        Segment_t *Segment)
{
    //---------------------------
    SeStatus RunStatus;
    //---------------------------
    
    LogStartOfNewSegment(SegmentList->Engine, Command, Segment);
    
    if (Command->Lock){
        Command->Lock(  SegmentList,
                        Segment->Bits.Data, 
                        Command->UserData,
                        Segment->Extension);
    }

    RunStatus = Command->Start( SegmentList,
                                Segment->Bits.Data, 
                                Command->UserData,
                                Segment->Extension);

    if (Command->Unlock){
        Command->Unlock(    SegmentList,
                            Segment->Bits.Data, 
                            Command->UserData,
                            Segment->Extension);
    }

    return RunStatus;
}



static SeStatus 
RunSegmentWithLocks(    SegmentList_t *SegmentList,
                        SegmentCommand_t *Command,
                        Segment_t *Segment)
{
    //---------------------------
    SeStatus RunStatus;
    //---------------------------
    
    if (Command->Lock){
        Command->Lock(  SegmentList,
                        Segment->Bits.Data, 
                        Command->UserData,
                        Segment->Extension);
    }

    RunStatus = Command->Run(   SegmentList,
                                Segment->Bits.Data, 
                                Command->UserData,
                                Segment->Extension);

    if (Command->Unlock){
        Command->Unlock(    SegmentList,
                            Segment->Bits.Data, 
                            Command->UserData,
                            Segment->Extension);
    }

    return RunStatus;
}



//
//  Implements "OR" semantics on chained conditionals.
//
static SeStatus
RunChainedConditionals(SegmentList_t *SegmentList,
                        Segment_t *ConditionalSegment)
{
    //-------------------------------------
    SeStatus Status;
    SegmentCommand_t *ConditionalCommand;
    //-------------------------------------

    for (;;){

        ConditionalCommand = ConditionalSegment->Command;

        //
        //  We should only be here if we are a conditional.
        //
        ASSERT(ConditionalCommand->SegmentFlags & ScfAbortConditional);

        Status = RunSegmentWithLocks(   SegmentList,
                                        ConditionalCommand,
                                        ConditionalSegment);
        //
        //  Conditional signaled we are done.
        //
        if (Status == SeAbortIfConditionTrue){
            return SeAbortIfConditionTrue;
        }
        //
        //  This conditional said we need to keep going, but check for 
        //  additional conditionals and of they are ok too.
        //
        else if (Status == SeAbortIfConditionFalse){

            if (ConditionalSegment->Conditional){
                ConditionalSegment = ConditionalSegment->Conditional;
                continue;
            }
            else{
                return SeAbortIfConditionFalse;
            }
        }
        //
        //  Unexpected status, just get out.
        //
        else{
            return Status;
        }
    }
}



static EngineRunStatus
ExecuteSegments(    SegmentList_t *SegmentList,
                    bool StartNewSegment
                    )
{
    //-----------------------------------------------------------------------
    SegmentCommand_t *Command;
    Segment_t *Segment;

    int TotalNumSegments;
    SeStatus RunStatus;
    //-----------------------------------------------------------------------

    TotalNumSegments = (*SegmentList->Segments).size();

    do {
        //
        //  Get the data needed to run the next step...
        //
        Segment = (*SegmentList->Segments)[SegmentList->CurIndex];
        Command = Segment->Command;


        //
        //  START OF NEW SEGMENT --------------------------------------------
        //
        if (StartNewSegment){

            //
            //  If we are starting a segment, we do not need to check 
            //  for Segment->Conditional, because a conditional is 
            //  instant, so we "just" checked it a ms ago.
            //
            RunStatus = StartSegmentWithLocks(  SegmentList,
                                                Command,
                                                Segment);
        }
        //
        //  CONTINUE RUNNING SEGMENT ----------------------------------------
        //
        else if (Command->Run){

            if (Segment->Conditional){

                RunStatus = RunChainedConditionals(SegmentList, Segment->Conditional);

                if (RunStatus == SeAbortIfConditionTrue){
                    // 
                    //  @todo - Do we need to log that we are skipping the Segment because of the 
                    //          Abort conditional?
                    //

                    //
                    //  This is unexpected, abnormal termination from the 
                    //  segment's point .  Give the segmenta chance to 
                    //  clean up.
                    //
                    if (Command->Stop){

                        Command->Stop(  SegmentList,
                                        Segment->Bits.Data, 
                                        Command->UserData, 
                                        Segment->Extension);
                    }
                }
                else if (RunStatus == SeAbortIfConditionFalse){
            
                    RunStatus = RunSegmentWithLocks(    SegmentList,
                                                        Command,
                                                        Segment);
                }
                else{
                    //
                    //  ERROR - Unknown return code from Abort segment.
                    //
                    return EngRunStatTerminate;
                }
            }
            else {
                RunStatus = RunSegmentWithLocks(SegmentList,
                                                Command,
                                                Segment);
            }
        }
        //
        //  ERROR -----------------------------------------------------------
        //  We are not starting a segment and there isn't a Run callback!
        //
        else{
            return EngRunStatTerminate;
        }

        switch(RunStatus){
            //
            //  The segment finished.  Go on to the next one.
            //
            case SeSuccess:
                StartNewSegment = true;
                DeleteChainedConditionals(Segment);
                SegmentList->CurIndex++;
                break;

            //
            //  The segment isn't done, we need to start calling the 
            //  Run() callback.
            //
            case SeContinue:
                StartNewSegment = false;
                break;

            //
            //  Someone wants to modify the execution order.
            //
            case SeActiveSegmentIndexModified:
                ASSERT(Segment->Command->SegmentFlags & ScfRepeat);
                StartNewSegment = true;
                DeleteChainedConditionals(Segment);
                SegmentList->CurIndex = SegmentList->BranchIndex;
                break;

            //
            //  An Abort conditional _start_ callback evaluated true, 
            //  skip any and all sequential conditionals.
            //
            case SeAbortIfConditionSkip:
                ASSERT(Segment->Command->SegmentFlags & ScfAbortConditional);
                StartNewSegment = true;
                DeleteChainedConditionals(Segment);

                //
                //  Skip all of the remaining Conditional segments until we find 
                //  the first that isn't a conditional.
                //
                while (Segment->Command->SegmentFlags & ScfAbortConditional){

                    SegmentList->CurIndex++;
                    //
                    //  Need to check here, in case someone creates an Abort conditional
                    //  at the end of a segment list, so we don't index off the vector.
                    //
                    if (SegmentList->CurIndex >= TotalNumSegments){
                        return EngRunStatEnd;
                    }
                    else {
                        Segment = (*SegmentList->Segments)[SegmentList->CurIndex];
                    }
                }
                //
                //  And then skip the segment that all of these abort segments 
                //  were attached to.
                //
                SegmentList->CurIndex++;
                break;

            //
            //  An Abort conditional start callback evaluated false, 
            //  tie the conditional to the next segment.
            //
            case SeAbortIfConditionAttach:
                ASSERT(Segment->Command->SegmentFlags & ScfAbortConditional);
                StartNewSegment = true;
                SegmentList->CurIndex++;

                //
                //  Need to check here, in case someone creates an Abort conditional
                //  at the end of a segment list, so we don't index off the vector.
                //
                if (SegmentList->CurIndex >= TotalNumSegments){
                    return EngRunStatEnd;
                }
                else {
                    (*SegmentList->Segments)[SegmentList->CurIndex]->Conditional = Segment;
                }
                break;

            //
            //  This was an Abort Segment that evaluated to true.
            //
            case SeAbortIfConditionTrue:
                StartNewSegment = true;
                DeleteChainedConditionals(Segment);
                SegmentList->CurIndex++;
                break;

            //
            //  ERROR - something went wrong, we are done with the experiment.
            //
            default:
                DeleteChainedConditionals(Segment);
                return EngRunStatTerminate;
        }


        //
        //  If we are at the end, we are done.
        //
        if (SegmentList->CurIndex >= TotalNumSegments){
            return EngRunStatEnd;
        }

    //
    //  Keep going while we have segments to start...
    //
    } while (StartNewSegment);


    return EngRunStatContinue;
}



/**
 *  Find the new index given where we are now, using IDs.
 */
static int 
FindNewSegmentIndex(    SegmentList_t *CurSegmentList, 
                        SegmentList_t *NewSegmentList)
{
    //----------------------------------------------------------------------
    Segment_t *Segment;
    unsigned int CurSegmentId;
    //----------------------------------------------------------------------

    //
    //  Find the ID of the active Segment...
    //
    Segment = (*CurSegmentList->Segments)[CurSegmentList->CurIndex];
    CurSegmentId = Segment->Bits.UniqueTag;

    //
    //  Find it in the new list, so we know where we'll start.
    //
    for (unsigned int i = 0; i<NewSegmentList->Segments->size(); i++){

        Segment = (*NewSegmentList->Segments)[i];

        //
        //  Found it!  Return the index.
        //
        if (Segment->Bits.UniqueTag == CurSegmentId){
            return i;
        }
    }

    //
    //  Bad edit, return -1.
    //
    return -1;
}



/**
 *  
 */
static void 
ImportSegmentExtensions(SegmentList_t *CurSegmentList, 
                        SegmentList_t *NewSegmentList)
{
    //----------------------------------------------------------------------
    Segment_t *CurSegment;
    Segment_t *NewSegment;
    SegmentCommand_t *Command;
    bool Found;
    //----------------------------------------------------------------------

    for (unsigned int i = 0; i<CurSegmentList->Segments->size(); i++){

        CurSegment = (*CurSegmentList->Segments)[i];

        //
        //  No extension, keep looking
        //
        if (CurSegment->Extension == NULL){
            continue;
        }

        //
        //  If there is an extension, we need to copy it, or clean it up.
        //
        Found = false;
        for (unsigned int j = 0; j<NewSegmentList->Segments->size(); j++){

            NewSegment = (*NewSegmentList->Segments)[j];

            if (NewSegment->Bits.UniqueTag == CurSegment->Bits.UniqueTag){
                NewSegment->Extension = CurSegment->Extension;
                CurSegment->Extension = NULL;
                Found = true;
                break;
            }
        }

        //
        //  We didn't find it in the new list, clean it up then.
        //
        if (!Found){
            Command = CurSegment->Command;

            if (Command->Cleanup != NULL) {
                Command->Cleanup(   CurSegmentList,
                                    CurSegment->Bits.Data, 
                                    Command->UserData,
                                    CurSegment->Extension);
            }
        }
    }


    //
    //  We need a pass through the new list, to init Extension data 
    //  for any added Segments.
    //
    for (unsigned int j = 0; j<NewSegmentList->Segments->size(); j++){

        NewSegment = (*NewSegmentList->Segments)[j];

        //
        //  If we have an extension, we copied it, keep looking.
        //
        if (NewSegment->Extension){
            continue;
        }

        Found = false;
        for (unsigned int i = 0; i<CurSegmentList->Segments->size(); i++){

            CurSegment = (*CurSegmentList->Segments)[i];

            if (NewSegment->Bits.UniqueTag == CurSegment->Bits.UniqueTag){
                Found = true;
                break;
            }
        }

        //
        //  This is a new segment, give it a chance to create an extension.
        //
        if (!Found){

            Command = NewSegment->Command;

            if (Command->Setup != NULL){
                NewSegment->Extension = Command->Setup( NewSegmentList,
                                                        NewSegment->Bits.Data, 
                                                        Command->UserData
                                                        );
            }
        }
    }
}



static void *
SegmentEngineThread(void *Parameter)
{
    //---------------------------------------------------------------------
    int rc;
    int Message;
    int Length;
    int Delay = -1;

    union {
        unsigned char RawBuffer[1];
        SegmentList_t *SegmentList;
    }Data;


    enum EngineState_t {

        Idle        = 0x10000,
        Running     = 0x20000,

    } EngineState = Idle;


    EngineRunStatus RunStatus = EngRunStatInvalid;
    bool StartNewSegment = true;
    int NewIndex;

    Engine_t *Engine;
    //---------------------------------------------------------------------

    Engine = (Engine_t *)Parameter;

    ASSERT(Engine->Signature == ENGINE_SIGNATURE);

    //
    //  We do not need cleanup.
    //
    pthread_detach(pthread_self());

    //
    //  If there is a real value in here, attempt to change 
    //  the Priority of the Workqueue.
    //
    if (Engine->RequestedThreadPriority >= 0){

        struct sched_param SchedulerParameters;
        //static int MaxFifoSchedPriority = 99;
        //static int MinFifoSchedPriority = 1;
        
        memset(&SchedulerParameters, 0, sizeof(SchedulerParameters));

        SchedulerParameters.sched_priority = Engine->RequestedThreadPriority;

        rc = sched_setscheduler (   0,
                                    SCHED_FIFO,
                                    &SchedulerParameters
                                    );
        (void)rc;
    }

    //
    //  Main thread loop
    //
    while (1){

        Length = sizeof(Data);

        rc = MqDequeueMessage(  Engine->SegmentThreadQueue, 
                                &Message, 
                                &Length, 
                                Data.RawBuffer, 
                                Delay);

        if (rc != 0){
            Message = EngMsgTimeout;
        }

        switch(Message + EngineState){
            //
            //  -------------------------------------------------------------
            //
            case EngMsgStart + Idle:

                EngineState = Running;
                Delay = SEGMENT_ENGINE_THREAD_RUN_PERIOD;

                //
                //  Take the lock on the SegmentList
                //
                pthread_mutex_lock(&Data.SegmentList->InUse);

                Engine->ActiveSegmentList = Data.SegmentList;
                Engine->ActiveSegmentList->CurIndex = 0;
                StartNewSegment = true;
                CompleteSyncMessage(Engine, EngMsgStart, SeSuccess);
                SetupAllSegments(Engine->ActiveSegmentList);
                break;

            case EngMsgStart + Running:
                CompleteSyncMessage(Engine, EngMsgStart, SeCommandIgnored);
                break;

			//
			//  -------------------------------------------------------------
			//
			case EngMsgStartInternal + Idle:

				EngineState = Running;
				Delay = SEGMENT_ENGINE_THREAD_RUN_PERIOD;

				//
				//  Take the lock on the SegmentList
				//
                pthread_mutex_lock(&Data.SegmentList->InUse);

				Engine->ActiveSegmentList = Data.SegmentList;
				Engine->ActiveSegmentList->CurIndex = 0;
				StartNewSegment = true;
				SetupAllSegments(Engine->ActiveSegmentList);
				break;

			case EngMsgStartInternal + Running:
				break;

			//
            //  -------------------------------------------------------------
            //
            case EngMsgTimeout + Running:

                RunStatus = ExecuteSegments(Engine->ActiveSegmentList, StartNewSegment);

                if (RunStatus == EngRunStatContinue){
                    StartNewSegment = false;
                }
                else{
                    MqQueueMessage(Engine->SegmentThreadQueue, EngMsgStopInternal, 0, NULL);
                }

                break;

            case EngMsgTimeout + Idle:
                break;

            //
            //  -------------------------------------------------------------
            //  Clean up and reset.
            case EngMsgStop + Running:

                StopActiveSegment(Engine->ActiveSegmentList);
                CleanupAllSegments(Engine->ActiveSegmentList);
                Engine->ActiveSegmentList->CurIndex = -1;
                pthread_mutex_unlock(&Engine->ActiveSegmentList->InUse);

                Engine->StaticCommandTable.End(Engine->StaticCommandTable.EndUserData);

                EngineState = Idle;
                Delay = -1;

                break;

            case EngMsgStop + Idle:
                break;


            //
            //  -------------------------------------------------------------
            //  Clean up and reset.
            case EngMsgStopInternal + Running:

                StopActiveSegment(Engine->ActiveSegmentList);
                CleanupAllSegments(Engine->ActiveSegmentList);
                Engine->ActiveSegmentList->CurIndex = -1;
                pthread_mutex_unlock(&Engine->ActiveSegmentList->InUse);

                if (RunStatus == EngRunStatEnd){
                    Engine->StaticCommandTable.End(Engine->StaticCommandTable.EndUserData);
                }
                else{
                    Engine->StaticCommandTable.Terminate(Engine->StaticCommandTable.TerminateUserData);
                }

                EngineState = Idle;
                Delay = -1;
                break;

            case EngMsgStopInternal + Idle:
                break;


            //
            //  -------------------------------------------------------------
            //  Clean up and reset on an external error.
            case EngMsgTerminate + Running:

                StopActiveSegment(Engine->ActiveSegmentList);
                CleanupAllSegments(Engine->ActiveSegmentList);
                Engine->ActiveSegmentList->CurIndex = -1;
                pthread_mutex_unlock(&Engine->ActiveSegmentList->InUse);

                Engine->StaticCommandTable.Terminate(Engine->StaticCommandTable.TerminateUserData);

                EngineState = Idle;
                Delay = -1;
                break;

            case EngMsgTerminate + Idle:
                break;


            //
            //  -------------------------------------------------------------
            //  Change what we are doing in mid-stream
            //
            case EngMsgModifySegmentList + Running:

                //
                //  Make sure we can line up the old with the new.
                //
                NewIndex = FindNewSegmentIndex(Engine->ActiveSegmentList, Data.SegmentList);
                if (NewIndex < 0){
                    CompleteSyncMessage(Engine, EngMsgModifySegmentList, SeDeletedCurrentSegment);
                    break;
                }

                ImportSegmentExtensions(Engine->ActiveSegmentList, Data.SegmentList);

                //
                //  Swap locks on the SegmentLists
                //
                pthread_mutex_unlock(&Engine->ActiveSegmentList->InUse);
                pthread_mutex_lock(&Data.SegmentList->InUse);

                Engine->ActiveSegmentList->CurIndex = -1;

                Engine->ActiveSegmentList = Data.SegmentList;
                Engine->ActiveSegmentList->CurIndex = NewIndex;

                ModifyActiveSegment(Engine->ActiveSegmentList);

                CompleteSyncMessage(Engine, EngMsgModifySegmentList, SeSuccess);
                break;

            case EngMsgModifySegmentList + Idle:
                CompleteSyncMessage(Engine, EngMsgModifySegmentList, SeCommandIgnored);
                break;


            //
            //  -------------------------------------------------------------
            //  Skip to the next segment.  We need to "start" it.
            //
            case EngMsgGotoNextSegment + Running:

                RunStatus = SkipSegment(Engine->ActiveSegmentList);

                if (RunStatus == EngRunStatContinue){
                    StartNewSegment = true;
                }
                else{
                    MqQueueMessage(Engine->SegmentThreadQueue, EngMsgStopInternal, 0, NULL);
                }

                CompleteSyncMessage(Engine, EngMsgGotoNextSegment, SeSuccess);
                break;

            case EngMsgGotoNextSegment + Idle:
                CompleteSyncMessage(Engine, EngMsgGotoNextSegment, SeCommandIgnored);
                break;


            //
            //  -------------------------------------------------------------
            //  Did we miss something?!
            //
            default:
                printf("Skipping unhandled [state, message] = [%d, %d]", EngineState, Message);
                break;
        }
    }

    return 0;
}


/**
 *  Cleans up an Engine data structure
 *  @todo - Do we need to delete an engine once it's created?
 */
void
TerminateSegmentExecutionEngine(Engine_t *Engine)
{
    if (Engine){

        pthread_mutex_destroy(&Engine->Lock);

        if (Engine->SegmentThreadQueue){
            MqDestroyMessageQueue(Engine->SegmentThreadQueue);
            Engine->SegmentThreadQueue = NULL;
        }

        for (int i = 0; i<NumSegmentEngineMessages; i++){
            sem_destroy(&Engine->Messages[i].hWait);
        }

        if (Engine->SegmentCommandTable){
            delete Engine->SegmentCommandTable;
        }

    }
}


ENGINE_HANDLE __attribute__  ((visibility ("default")))
SeCreateSegmentEngine(int ThreadPriority)
{
    //--------------------
    Engine_t *Engine;
    pthread_mutexattr_t attr;
    int rc;
    //--------------------

    Engine = (Engine_t *)malloc(sizeof (Engine_t));

    if (!Engine){
        return NULL;
    }

    memset(Engine, 0, sizeof(Engine_t));

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&Engine->Lock, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);

    Engine->SegmentThreadQueue = MqCreateMessageQueue();
    if (!Engine->SegmentThreadQueue){
        TerminateSegmentExecutionEngine(Engine);
        return NULL;
    }

    for (int i = 0; i<NumSegmentEngineMessages; i++){
        rc = sem_init(&Engine->Messages[i].hWait, 0, 0);
        ASSERT(rc == 0);
    }

    Engine->SegmentCommandTable = new map<unsigned int, SegmentCommand_t*>();

    Engine->Signature = ENGINE_SIGNATURE;
    Engine->RequestedThreadPriority = ThreadPriority;


    //
    //  Create a worker thread...
    //
    rc = pthread_create(&Engine->ThreadHandle, NULL, SegmentEngineThread, Engine);
    ASSERT(rc == 0);

    RegisterRepeatSegment(Engine);

    LogVersion();

    return (ENGINE_HANDLE)Engine;
}


