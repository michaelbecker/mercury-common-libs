#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>



#include "SegmentExecutionEngine.h"
#include "SegmentExecutionEnginePrivate.h"
#include "TaWinAssert.h"

#include "SegmentBits.h"
#include "CommonSegments.h"


int __attribute__  ((visibility ("default")))
SeFindNextSegmentIfUserFlags(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                UserFlagPredicate Predicate,
                                unsigned int DesiredUserFlags, 
                                unsigned int *Id)
{
    //----------------------------
    SegmentCommand_t *Command;
    Segment_t *Segment;
    int TotalNumSegments;
    int WorkingSegmentIndex;
    SegmentList_t  *SegmentList;
    //----------------------------

    ASSERT(SegmentListHandle != NULL);
    SegmentList = (SegmentList_t *)SegmentListHandle;
    ASSERT(SegmentList->Signature == SEGMENT_LIST_SIGNATURE);

    TotalNumSegments = (*SegmentList->Segments).size();
    if (SegmentList->CurIndex >= TotalNumSegments){
        return 0;
    }
    
    for (int s = 0; s<TotalNumSegments; s++){

        Segment = (*SegmentList->Segments)[s];
        Command = Segment->Command;

        if (Command->InitSearch){
            Command->InitSearch(    SegmentListHandle, 
                                    Segment->Bits.Data, 
                                    Command->UserData, 
                                    Segment->Extension);
        }
    }

    WorkingSegmentIndex = SegmentList->CurIndex;
    WorkingSegmentIndex++;

    while (WorkingSegmentIndex < TotalNumSegments){

        Segment = (*SegmentList->Segments)[WorkingSegmentIndex];
        Command = Segment->Command;

        int rc = Predicate(DesiredUserFlags, Command->UserFlags);
        if (rc){
            *Id = Command->Id;
            return 1;
        }

        if (Command->InitSearch){

            SeStatus IsModified;
            int NewSegment;

            IsModified = Command->ModifyIndexForSearch( SegmentListHandle, 
                                                        Segment->Bits.Data, 
                                                        Command->UserData, 
                                                        Segment->Extension,
                                                        &NewSegment);

            if (IsModified == SeActiveSegmentIndexModified){
                WorkingSegmentIndex = NewSegment;
            }
        }
    }

    return 0;
}



static int 
PredicateUserFlagsEqual(unsigned int DesiredUserFlags, unsigned int UserFlags)
{
    if (DesiredUserFlags == UserFlags)
        return 1;
    else
        return 0;
}


static int 
PredicateUserFlagsNotEqual(unsigned int DesiredUserFlags, unsigned int UserFlags)
{
    if (DesiredUserFlags != UserFlags)
        return 1;
    else
        return 0;
}


static int 
PredicateUserFlagsAnyBitSet(unsigned int DesiredUserFlags, unsigned int UserFlags)
{
    if (DesiredUserFlags & UserFlags)
        return 1;
    else
        return 0;
}


static int 
PredicateUserFlagsAllBitsSet(unsigned int DesiredUserFlags, unsigned int UserFlags)
{
    if ((DesiredUserFlags & UserFlags) == DesiredUserFlags)
        return 1;
    else
        return 0;
}


int __attribute__  ((visibility ("default")))
SeFindNextSegmentIfUserFlagsEqual(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                    unsigned int DesiredUserFlags,
                                    unsigned int *Id)
{
    int Success = SeFindNextSegmentIfUserFlags( SegmentListHandle,
                                                PredicateUserFlagsEqual,
                                                DesiredUserFlags,
                                                Id);
    return Success;
}


int __attribute__  ((visibility ("default")))
SeFindNextSegmentIfUserFlagsNotEqual(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                        unsigned int DesiredUserFlags,
                                        unsigned int *Id)
{
    int Success = SeFindNextSegmentIfUserFlags( SegmentListHandle,
                                                PredicateUserFlagsNotEqual,
                                                DesiredUserFlags,
                                                Id);
    return Success;
}


int __attribute__  ((visibility ("default")))
SeFindNextSegmentIfUserFlagsAnyBitSet(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                        unsigned int DesiredUserFlags,
                                        unsigned int *Id)
{
    int Success = SeFindNextSegmentIfUserFlags( SegmentListHandle,
                                                PredicateUserFlagsAnyBitSet,
                                                DesiredUserFlags,
                                                Id);
    return Success;
}



int __attribute__  ((visibility ("default")))
SeFindNextSegmentIfUserFlagsAllBitsSet( SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                        unsigned int DesiredUserFlags,
                                        unsigned int *Id)
{
    int Success = SeFindNextSegmentIfUserFlags( SegmentListHandle,
                                                PredicateUserFlagsAllBitsSet,
                                                DesiredUserFlags,
                                                Id);
    return Success;
}



