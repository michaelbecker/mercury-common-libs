#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>



#include "SegmentExecutionEngine.h"
#include "SegmentExecutionEnginePrivate.h"

#include "SegmentBits.h"
#include "CommonSegments.h"


#include "TaWinAssert.h"


/**
 *  Internal repeat type to track loops.
 */
struct Repeat_t {

    unsigned int CurrentCount;
    bool Complete;

    //
    //  Temporary variables to allow someone to search the List 
    //  taking repeats into account.
    //
    struct {

        unsigned int CurrentCount;
        bool Complete;

    } PredicateScratch;
};



/**
 *  This is _ONLY_ for predicate search functions.
 */
void __attribute__  ((visibility ("default")))
RepeatInitSearchCallback(   SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                            unsigned char *Data, 
                            void *UserData, 
                            void *Extension)
{
    //------------------------------------------
    (void)SegmentListHandle;
    (void)Data;
    (void)UserData;
    Repeat_t *Repeat = (Repeat_t *)Extension;
    //------------------------------------------

    Repeat->PredicateScratch.Complete = Repeat->Complete;
    Repeat->PredicateScratch.CurrentCount = Repeat->CurrentCount;
}



SeStatus __attribute__  ((visibility ("default")))
RepeatModifyIndexForSearch( SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                            unsigned char *Data, 
                            void *UserData, 
                            void *Extension,
                            int *AdjustedWorkingIndex)
{
    //-----------------------------------------------------
    (void)SegmentListHandle;
    (void)UserData;

    Repeat_t *Repeat = (Repeat_t *)Extension;
    CsRepeatData_t *RepeatData = (CsRepeatData_t *)Data;
    //-----------------------------------------------------

    //
    //  We are nested, reset the Repeat.
    //
    if (Repeat->PredicateScratch.Complete){
        Repeat->PredicateScratch.Complete = false;
        Repeat->PredicateScratch.CurrentCount = 0;
    }

    //
    //  looping == changing an index
    //
    if (Repeat->PredicateScratch.CurrentCount < RepeatData->Count){
        Repeat->PredicateScratch.CurrentCount++;
        *AdjustedWorkingIndex = RepeatData->Index;
        return SeActiveSegmentIndexModified;
    }
    else{
        Repeat->PredicateScratch.Complete = true;
        return SeSuccess;
    }
}



SeStatus __attribute__  ((visibility ("default"))) 
RepeatStartCallback(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData,
                        void *Extension)
{
    //-----------------------------------------------------
    (void)UserData;
    Repeat_t *Repeat = (Repeat_t *)Extension;
    CsRepeatData_t *RepeatData = (CsRepeatData_t *)Data;
    //-----------------------------------------------------

    //
    //  We are nested, reset the Repeat.
    //
    if (Repeat->Complete){
        Repeat->Complete = false;
        Repeat->CurrentCount = 0;
    }

    //
    //  looping == changing an index
    //
    if (Repeat->CurrentCount < RepeatData->Count){
        Repeat->CurrentCount++;
        SeSetIndexForBranch(SegmentListHandle, RepeatData->Index);
        return SeActiveSegmentIndexModified;
    }
    else{
        Repeat->Complete = true;
        return SeSuccess;
    }
}



/**
 *  UserData == SegmentList, special code to do this.
 */
SeStatus __attribute__  ((visibility ("default")))
RepeatValidateCallback( SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData)
{
    //-----------------------------------------------------
    (void)UserData;
    CsRepeatData_t *RepeatData = (CsRepeatData_t *)Data;
    //-----------------------------------------------------

    unsigned int TotalNumSegments = SeGetTotaltNumSegments(SegmentListHandle);

    //
    //  We shouldn't walk off the end.
    //
    if (RepeatData->Index >= TotalNumSegments){
		return SeInvalid;
    }

    int MySegmentIndex = SeGetCurrentSegmentIndex(SegmentListHandle);

    //
    //  Prevent infinite loops and looping "ahead" of ourselves.
    //
    if ((unsigned int)MySegmentIndex <= RepeatData->Index){
		return SeInvalid;
    }

    //
    //  Counts of zero will get flagged for now.
    //
    if (RepeatData->Count == 0){
		return SeInvalid;
    }
    else{
		return SeSuccess;
    }
}



void __attribute__  ((visibility ("default"))) 
RepeatToStringCallback( SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData, 
                        void *Extension,
                        char *StringBuffer, 
                        int *MaxStringBufferLength)
{
    //-----------------------------------------------------
    (void)UserData;
    (void)SegmentListHandle;
    int Length;
    Repeat_t *Repeat = (Repeat_t *)Extension;
    CsRepeatData_t *RepeatData = (CsRepeatData_t *)Data;
    //-----------------------------------------------------

    if (Repeat->CurrentCount >= RepeatData->Count){
        Length = snprintf(StringBuffer, *MaxStringBufferLength, 
            "REPEAT %d COMPLETE (%d of %d)", 
            RepeatData->Index + 1, 
            Repeat->CurrentCount,
            RepeatData->Count
            );
        *MaxStringBufferLength = Length;
    }
    else{
        Length = snprintf(StringBuffer, *MaxStringBufferLength, 
            "REPEAT %d (%d of %d)", 
            RepeatData->Index + 1, 
            Repeat->CurrentCount,
            RepeatData->Count
            );
        *MaxStringBufferLength = Length;
    }
}



void *
RepeatSetupCallback(SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                    unsigned char *Data, 
                    void *UserData)
{
    //-----------------------------------------------------
    (void)Data;
    (void)UserData;
    (void)SegmentListHandle;
    Repeat_t *Repeat;
    //-----------------------------------------------------

    Repeat = (Repeat_t *)malloc(sizeof(Repeat_t));
    ASSERT(Repeat != NULL);

    Repeat->Complete = false;
    Repeat->CurrentCount = 0;

    return Repeat;
}



void __attribute__  ((visibility ("default"))) 
RepeatCleanupCallback(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        unsigned char *Data, 
                        void *UserData, 
                        void *Extension)
{
    //-----------------------------------------------------
    (void)Data;
    (void)UserData;
    (void)SegmentListHandle;
    //-----------------------------------------------------

    Repeat_t *Repeat = (Repeat_t *)Extension;
    free(Repeat);
}



void
RegisterRepeatSegment(Engine_t *Engine)
{
    SegmentCommand_t Command = {

        CsRepeat,                   //  SubCommand

        ScfRepeat,                  //  SegmentFlags
        0,                          //  UserFlags
        sizeof(CsRepeatData_t),     //  DataLength
        NULL,                       //  UserData

        RepeatStartCallback,        //  Start function
        RepeatToStringCallback,     //  ToString function
        NULL,                       //  Run function
        NULL,                       //  Stop function
        NULL,                       //  Modify function
        RepeatValidateCallback,     //  Validate function
        RepeatSetupCallback,        //  Setup function
        RepeatCleanupCallback,      //  Cleanup function
        NULL,                       //  Lock
        NULL,                       //  Unlock
        RepeatInitSearchCallback,   //  InitSearch
        RepeatModifyIndexForSearch, //  ModifyIndexForSearch
    };

    SeRegisterSegmentCommand(Engine, &Command);
}
