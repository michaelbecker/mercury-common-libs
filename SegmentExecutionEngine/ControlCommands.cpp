#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:


#include "TaWinAssert.h"
#include "SegmentExecutionEngine.h"
#include "SegmentExecutionEnginePrivate.h"

using namespace std;


void __attribute__  ((visibility ("default")))
SeSetIndexForBranch(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        int NewIndex)
{
    //------------------------------------------
    SegmentList_t  *SegmentList;
    //------------------------------------------

    SegmentList = (SegmentList_t *)SegmentListHandle;
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    if ((unsigned int)NewIndex >= SegmentList->Segments->size()){
        return;
    }
    else if (NewIndex == SegmentList->CurIndex){
        return;
    }
    else {
        SegmentList->BranchIndex = NewIndex;
    }
}



unsigned int __attribute__  ((visibility ("default")))
SeGetTotaltNumSegments( SE_SEGMENT_LIST_HANDLE SegmentListHandle
                        )
{
    //------------------------------------------
    SegmentList_t  *SegmentList;
    //------------------------------------------

    SegmentList = (SegmentList_t *)SegmentListHandle;
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    return SegmentList->Segments->size();
}



int __attribute__  ((visibility ("default")))
SeGetCurrentSegmentIndex(   SE_SEGMENT_LIST_HANDLE SegmentListHandle
                            )
{
    //------------------------------------------
    SegmentList_t  *SegmentList;
    //------------------------------------------

    SegmentList = (SegmentList_t *)SegmentListHandle;
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    return SegmentList->CurIndex;
}



int __attribute__  ((visibility ("default")))
SeStartSegmentList(SE_SEGMENT_LIST_HANDLE SegmentListHandle)
{
    //-----------------------------------
    SegmentList_t  *SegmentList;
    int Status;
    //-----------------------------------

    ASSERT(SegmentListHandle != NULL);
    SegmentList = (SegmentList_t *)SegmentListHandle;
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    Status = SendSyncMessage(   SegmentList, 
                                EngMsgStart, 
                                (unsigned char *)&SegmentList, 
                                sizeof(SegmentList_t *));

    return Status;
}



/**
 *	Only call from End or Terminate callback.
 */
void __attribute__  ((visibility ("default")))
SeStartSegmentListFromEndCallback(SE_SEGMENT_LIST_HANDLE SegmentListHandle)
{
	//-----------------------------------
	SegmentList_t  *SegmentList;
	Engine_t *Engine;
	//-----------------------------------

	ASSERT(SegmentListHandle != NULL);
	SegmentList = (SegmentList_t *)SegmentListHandle;
	ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

	Engine = SegmentList->Engine;

	MqQueueMessage(	Engine->SegmentThreadQueue, 
					EngMsgStartInternal, 
					sizeof(SegmentList_t *), 
					(unsigned char *)&SegmentList);
}



/**
 *  Returns a new HANDLE to use, or NULL.  
 */
SE_SEGMENT_LIST_HANDLE __attribute__  ((visibility ("default")))
SeModifySegmentList(    SE_SEGMENT_LIST_HANDLE OriginalListHandle,  /**< The list we are modifying */
                        unsigned char *Data,                        /**< Binary buffer of the Comm format of a SegmentList */
                        int Length,                                 /**< Length of Data */
                        SeStatus *ErrorStatus,                      /**< Status of Parse */
                        int *SegmentIndex                           /**< Index of Segment error, if applicable */
                        )
{
    //--------------------------------------
    SE_SEGMENT_LIST_HANDLE Handle;
    SegmentList_t  *OriginalSegmentList;
    SegmentList_t  *NewSegmentList;
    int Status;
    //--------------------------------------

    ASSERT(OriginalListHandle != NULL);
    OriginalSegmentList = (SegmentList_t *)OriginalListHandle;
    ASSERT(OriginalSegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    //
    //  Build the list against the engine of the current list.
    //
    Handle = SeCreateSegmentList(   OriginalSegmentList->Engine,
                                    Data,
                                    Length,
                                    ErrorStatus,
                                    SegmentIndex);
    //
    //  Invalid list
    //
    if (Handle == NULL){
        return NULL;
    }
    else {
        NewSegmentList = (SegmentList_t *)Handle;
        ASSERT(NewSegmentList->Signature == SEGMENT_LIST_SIGNATURE);
    }

    //
    //  Ask the Engine to change the executing list...
    //
    Status = SendSyncMessage(   NewSegmentList, 
                                EngMsgModifySegmentList, 
                                (unsigned char *)&NewSegmentList, 
                                sizeof(SegmentList_t *));

    //
    //  Engine said no, clean up and let the caller know...
    //
    if (Status != SeSuccess){
        SeDestroySegmentList(Handle);
        *ErrorStatus = (SeStatus)Status;
        *SegmentIndex = -1;
        return NULL;
    }
    //
    //  Otherwise, return the new handle.
    //
    else{
        return Handle;
    }
}



void __attribute__  ((visibility ("default")))
SeStopSegmentList(SE_SEGMENT_LIST_HANDLE SegmentListHandle)
{
    //-----------------------------------
    SegmentList_t  *SegmentList;
    //-----------------------------------
    
    ASSERT(SegmentListHandle != NULL);
    SegmentList = (SegmentList_t *)SegmentListHandle;
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    SendAsyncMessage(   SegmentList, 
                        EngMsgStop, 
                        NULL, 
                        0);
}



void __attribute__  ((visibility ("default")))
SeTerminateSegmentList(SE_SEGMENT_LIST_HANDLE SegmentListHandle)
{
    //-----------------------------------
    SegmentList_t  *SegmentList;
    //-----------------------------------
    
    ASSERT(SegmentListHandle != NULL);
    SegmentList = (SegmentList_t *)SegmentListHandle;
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    SendAsyncMessage(   SegmentList, 
                        EngMsgTerminate, 
                        NULL, 
                        0);

    return;
}



int __attribute__  ((visibility ("default")))
SeGotoNextSegment(SE_SEGMENT_LIST_HANDLE SegmentListHandle)
{
    //-----------------------------------
    SegmentList_t  *SegmentList;
    int Status;
    //-----------------------------------

    ASSERT(SegmentListHandle != NULL);
    SegmentList = (SegmentList_t *)SegmentListHandle;
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    Status = SendSyncMessage(   SegmentList, 
                                EngMsgGotoNextSegment, 
                                NULL,
                                0);

    return Status;
}


