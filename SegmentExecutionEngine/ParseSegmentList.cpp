#include <string.h>

#include "TaWinAssert.h"
#include "SegmentExecutionEngine.h"
#include "SegmentExecutionEnginePrivate.h"

#include "SegmentBits.h"
#include "CommonSegments.h"



/**
 *  Private fcn to free SegmentList_t and all associated 
 *  data structures.
 */
static void
DestroySegmentList(SegmentList_t *SegmentList)
{
    //-----------------------------------------
    Segment_t *Segment;
    int NumberSegments;
    //-----------------------------------------

    ASSERT(SegmentList != NULL);
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    //
    //  Wait until this is not being used anymore.
    //
    pthread_mutex_lock(&SegmentList->InUse);
    pthread_mutex_unlock(&SegmentList->InUse);
    pthread_mutex_destroy(&SegmentList->InUse);

    SegmentList->Signature = 0xEEEEEEEE;

    NumberSegments = SegmentList->Segments->size();

    for (int s = 0; s < NumberSegments; s++){

        Segment = (*SegmentList->Segments)[s];

        if (Segment != NULL){
            ASSERT(Segment->Signature == SEGMENT_SIGNATURE);
            Segment->Signature = 0xEEEEEEEE;
            free(Segment);
        }
    }

    delete (SegmentList->Segments);
    free(SegmentList);
}



static SegmentCommand_t *
InternalFindSegmentCommand( Engine_t *Engine,
                            unsigned int Id
                            )
{
    //----------------------------------------------------------------------
    map<unsigned int, SegmentCommand_t*>::iterator SegmentCommandIterator;
    SegmentCommand_t  *SegmentCommand;
    //----------------------------------------------------------------------

    SegmentCommandIterator = Engine->SegmentCommandTable->find(Id);
    if (SegmentCommandIterator == Engine->SegmentCommandTable->end()){
        return NULL;
    }


    SegmentCommand = SegmentCommandIterator->second;
    return SegmentCommand;
}



/**
 *  This is the only function that finds a SegmentCommand that corresponds 
 *  to a particular segment.
 */
static SegmentCommand_t *
FindSegmentCommand( Engine_t *Engine,
                    SegmentBits_t *SegmentData)
{
    //-----------------------------------
    SegmentCommand_t  *SegmentCommand;
    //-----------------------------------

    //
    //  Check the Segment Tag.
    //
    if (    SegmentData->SegmentTag[0] != 'S' ||
            SegmentData->SegmentTag[1] != 'G' ||
            SegmentData->SegmentTag[2] != 'M' ||
            SegmentData->SegmentTag[3] != 'T' ){
        return NULL;
    }

    SegmentCommand = InternalFindSegmentCommand(Engine, 
                                                SegmentData->Id);
    return SegmentCommand;
}



//
//  Check that there are no duplicated Segment Sequence numbers,
//  these are our semgent IDs.
//
static int
CheckForDuplicateSegmentIds(SegmentList_t *SegmentList)
{
    //---------------------------
    Segment_t *TailSegment;
    Segment_t *Segment;
    int CurrentListSize;
    //---------------------------

    CurrentListSize = SegmentList->Segments->size();
    TailSegment = (*SegmentList->Segments)[CurrentListSize - 1];

    //
    //  Walk the list, checking that the last Segment added doesn't 
    //  share a Sequencenumber with any other segment.
    //  
    for (int i = 0; i<(CurrentListSize - 1); i++){

        Segment = (*SegmentList->Segments)[i];
        
        if (Segment->Bits.UniqueTag == TailSegment->Bits.UniqueTag){
            return 1;
        }
    }

    return 0;
}



/**
 *  In case any of the validate calls need to look forward, let's do this 
 *  after we've parsed and built the entire list, so we have it all.
 */
static bool
ValidateAllSegments(SegmentList_t *SegmentList,
                    SeStatus *ErrorStatus,
                    int *SegmentIndex
                    )
{
    //----------------------------------------------------------------------
    Segment_t *Segment;
    SegmentCommand_t *Command;
	SeStatus Valid;
    //----------------------------------------------------------------------

    for (unsigned int i = 0; i<SegmentList->Segments->size(); i++){

        Segment = (*SegmentList->Segments)[i];
        Command = Segment->Command;

        if (Command->Validate != NULL) {

            //
            //  We need to guarantee that the Validate function can find 
            //  what it's position is, for looping segments.
            //
            SegmentList->CurIndex = i;

            Valid = Command->Validate(  SegmentList,
                                        Segment->Bits.Data, 
                                        Command->UserData);
			//
			//	If it didn't validate, pass it back up to Trios.
			//
			if (Valid != SeSuccess){
                *ErrorStatus = Valid;
                *SegmentIndex = i;
                return false;
            }
        }
    }

    return true;
}




SE_SEGMENT_LIST_HANDLE __attribute__  ((visibility ("default")))
SeCreateSegmentList(    ENGINE_HANDLE hEngine, 
                        unsigned char *Data, 
                        int Length,
                        SeStatus *ErrorStatus,
                        int *SegmentIndex)
{
    //----------------------------------------------------------------------
    map<unsigned int, SegmentCommand_t*>::iterator SegmentCommandIterator;

    SegmentList_t       *SegmentList;
    Segment_t           *Segment;
    SegmentCommand_t    *SegmentCommand;
    int                 DupFound;
    Engine_t            *Engine;
    SegmentBits_t       *SegmentData;
    pthread_mutexattr_t attr;
    int rc;
    //----------------------------------------------------------------------

    if (ErrorStatus == NULL){
        return NULL;
    }
    if (Data == NULL){
        *ErrorStatus = SeInvalidParameters;
        return NULL;
    }
    if (Length == 0){
        *ErrorStatus = SeInvalidParameters;
        return NULL;
    }
    if (SegmentIndex == NULL){
        *ErrorStatus = SeInvalidParameters;
        return NULL;
    }

    //
    //  Recover our engine object
    //
    Engine = (Engine_t *)hEngine;
    if (Engine->Signature != ENGINE_SIGNATURE){
        *ErrorStatus = SeInvalidParameters;
        return NULL;
    }

    //
    //  Start creating a SegmentList
    //
    SegmentList = (SegmentList_t *)malloc(sizeof(SegmentList_t));

    if (!SegmentList){
        *ErrorStatus = SeOutOfMemory;
        return NULL;
    }

    SegmentList->Signature = SEGMENT_LIST_SIGNATURE;
    SegmentList->Engine = Engine;

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&SegmentList->InUse, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);
    
    //
    //  Speed Optimization, reserve space up front.
    //
    int EstimatedCount = Length / (sizeof(SegmentBits_t) - 1);
    if (EstimatedCount <= 0){
        EstimatedCount = 1;
    }

    SegmentList->Segments = new vector<Segment_t *>();
    SegmentList->Segments->reserve(EstimatedCount);
    SegmentData = (SegmentBits_t *)Data;
    int Index = 0;

    while (Length > 0){

        //
        //  Find the corresponding SegmentCommand. We need this and the size
        //  stored in it too.
        //
        SegmentCommand = FindSegmentCommand(Engine, SegmentData);
        if (!SegmentCommand){
            *ErrorStatus = SeUnknownSegment;
            *SegmentIndex = Index;
            DestroySegmentList(SegmentList);
            return NULL;
        }

        //
        //  Get the length of the internal Segment data struct as well
        //  as the length of the Comm message we are parsing.
        //
        int MsgLength;

        MsgLength = sizeof(SegmentBits_t) - 1 + SegmentCommand->DataLength;

        //
        //  Real size of the segment.
        //
        int SegmentLength = sizeof(Segment_t) + SegmentCommand->DataLength - 1;

        //
        //  Adjusted to make allocation sizes more uniform for CE.
        //  We assume that most if not all segments have at most 3 data 
        //  attached to them.  But we handle it when they are bigger.
        //
        //  TODO - Revisit this if / when we write / find a SLAB allocator.
        //
        int AdjustedSegmentLength = (sizeof(Segment_t) - 1) + (3 * sizeof(int));
        int AllocationSize = (SegmentLength > AdjustedSegmentLength) ? SegmentLength : AdjustedSegmentLength;

        //
        //  Create the internal Segment_t themselves for this script.
        //
        Segment = (Segment_t *)malloc(AllocationSize);

        if(!Segment){
            *ErrorStatus = SeOutOfMemory;
            *SegmentIndex = Index;
            DestroySegmentList(SegmentList);
            return NULL;
        }

        memset(Segment, 0, SegmentLength);
        Segment->Signature = SEGMENT_SIGNATURE;
        Segment->Extension = NULL;
        Segment->Command = SegmentCommand;
        Segment->Conditional = NULL;
        Segment->BitsLength = MsgLength;

        //
        //  Fill in the segment bits, what Trios sent us.
        //
        memcpy(&Segment->Bits, SegmentData, MsgLength);

        //
        //  Add it to the list.
        //
        SegmentList->Segments->push_back(Segment);

        //
        //  Duplicate Sequence Ids are not allowed.
        //
        DupFound = CheckForDuplicateSegmentIds(SegmentList);

        if (DupFound){
            *ErrorStatus = SeDupSequenceNumber;
            *SegmentIndex = Index;
            DestroySegmentList(SegmentList);
            return NULL;
        }

        Index++;
        Length -= MsgLength;
        Data += MsgLength;
        SegmentData = (SegmentBits_t *)Data;
    }


    bool Valid = ValidateAllSegments( SegmentList, 
                                      ErrorStatus, 
                                      SegmentIndex);

    if (Valid){
        //
        //  Sanity check, there isn't one until we start executing the List.
        //
        SegmentList->CurIndex = -1;

        return (SE_SEGMENT_LIST_HANDLE)SegmentList;
    }
    else{
        DestroySegmentList(SegmentList);
        return NULL;
    }

}



void __attribute__  ((visibility ("default")))
SeDestroySegmentList(SE_SEGMENT_LIST_HANDLE SegmentListHandle)
{
    //------------------------------
    SegmentList_t  *SegmentList;
    //------------------------------

    if (SegmentListHandle){

        SegmentList = (SegmentList_t *)SegmentListHandle;
    
        if (SegmentList->Signature == SEGMENT_LIST_SIGNATURE){
            DestroySegmentList(SegmentList);
        }
    }
}



SE_SEGMENT_LIST_HANDLE __attribute__  ((visibility ("default")))
SeInternalCreateEmptySegmentList(	ENGINE_HANDLE hEngine,
									SeStatus *ErrorStatus )
{
	//--------------------------------
	Engine_t *Engine;
	SegmentList_t *SegmentList;
    pthread_mutexattr_t attr;
    int rc;
	//--------------------------------

    if (ErrorStatus == NULL){
        return NULL;
    }

	//
	//  Recover our engine object
	//
	Engine = (Engine_t *)hEngine;
	if (Engine->Signature != ENGINE_SIGNATURE){
		*ErrorStatus = SeInvalidParameters;
		return NULL;
	}

	//
	//  Start creating a SegmentList
	//
	SegmentList = (SegmentList_t *)malloc(sizeof(SegmentList_t));

	if (!SegmentList){
		*ErrorStatus = SeOutOfMemory;
		return NULL;
	}

	SegmentList->Signature = SEGMENT_LIST_SIGNATURE;
	SegmentList->Engine = Engine;
 
    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);

    rc = pthread_mutex_init(&SegmentList->InUse, &attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);
    
	SegmentList->Segments = new vector<Segment_t *>();
    SegmentList->Segments->reserve(20);

    //
    //  There isn't one.
    //
    SegmentList->CurIndex = -1;

    return (SE_SEGMENT_LIST_HANDLE)SegmentList;
}



SeStatus  __attribute__  ((visibility ("default")))
SeInternalAppendSegment(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                        unsigned int Id,
                        unsigned int UniqueTag,
                        unsigned char *Data,
                        int DataLength
                        )
{
    //-----------------------------------
    SegmentList_t  *SegmentList;
    SegmentCommand_t *SegmentCommand;
    //-----------------------------------

    ASSERT(SegmentListHandle != NULL);
    SegmentList = (SegmentList_t *)SegmentListHandle;
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    //
    //  Find the corresponding SegmentCommand. We need this and the size
    //  stored in it too.
    //
    SegmentCommand = InternalFindSegmentCommand(SegmentList->Engine, Id);

    ASSERT(SegmentCommand != NULL);
    ASSERT((unsigned int)DataLength == SegmentCommand->DataLength);

    //
    //  Real size of the segment.
    //
    int SegmentLength = sizeof(Segment_t)+SegmentCommand->DataLength - 1;

    //
    //  Adjusted to make allocation sizes more uniform for CE.
    //  We assume that most if not all segments have at most 3 data 
    //  attached to them.  But we handle it when they are bigger.
    //
    int AdjustedSegmentLength = (sizeof(Segment_t)-1) + (3 * sizeof(int));
    int AllocationSize = (SegmentLength > AdjustedSegmentLength) ? SegmentLength : AdjustedSegmentLength;

    //
    //  Create the internal Segment_t themselves for this script.
    //
    Segment_t *Segment = (Segment_t *)malloc(AllocationSize);

    if (!Segment){
        return SeOutOfMemory;
    }

    memset(Segment, 0, SegmentLength);
    Segment->Signature = SEGMENT_SIGNATURE;
    Segment->Extension = NULL;
    Segment->Command = SegmentCommand;
    Segment->Conditional = NULL;
    Segment->BitsLength = sizeof(SegmentBits_t) + DataLength - 1;


    //
    //  Fill in the type specific data, SequenceNumber and Data[]
    //
    Segment->Bits.SegmentTag[0] = 'S';
    Segment->Bits.SegmentTag[1] = 'G';
    Segment->Bits.SegmentTag[2] = 'M';
    Segment->Bits.SegmentTag[3] = 'T';
    Segment->Bits.Id = Id;
    Segment->Bits.UniqueTag = UniqueTag;
    memcpy(Segment->Bits.Data, Data, SegmentCommand->DataLength);

    //
    //  Add it to the list.
    //
    SegmentList->Segments->push_back(Segment);

    //
    //  Duplicate Sequence Ids are not allowed.
    //
    int DupFound = CheckForDuplicateSegmentIds(SegmentList);

    if (DupFound){
        SegmentList->Segments->pop_back();
        free(Segment);
        return SeDupSequenceNumber;
    }


    return SeSuccess;
}

