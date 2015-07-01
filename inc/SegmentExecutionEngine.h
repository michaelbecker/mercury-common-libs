#ifndef SEGMENT_EXECUTION_ENGINE_H__
#define SEGMENT_EXECUTION_ENGINE_H__

#include "SegmentBits.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 *  When you create an engine, you get a handle to refer to it.
 */
typedef void * ENGINE_HANDLE;

/**
 *  Handle to a processed, validated Segment List.
 */
typedef void * SE_SEGMENT_LIST_HANDLE;


/** 
 *  Call to create a segment execution script engine.  This returns 
 *  a handle to that specific engine.  This may be called multiple times
 *  to create multiple engines.
 *  
 *  @todo   Need to code a tear-down routine.  But not a high priority.
 *          Maybe make TerminateSegmentExecutionEngine() public, and add
 *          thread deletion code?
 */
ENGINE_HANDLE 
SeCreateSegmentEngine(int ThreadPriority);



/**
 *  Called on the normal end of a Segment List.  
 *  You reached the end.
 */
typedef void ( *EndCallback_t)(void *UserData);

/**
 *  Called on abnormal termination of a Segment List.  This may 
 *  be due to an error, or due to Trios telling us to stop.
 */
typedef void ( *TerminateCallback_t)(void *UserData);

/**
 *  Called with the start of every Segment.
 */
typedef void ( *SegmentChangeCallback_t)(  int CurrentSegmentIndex,        //  Zero based index into the Segment List.
                                                    SegmentBits_t *RawSegment,      //  Actual pointer to the data sent by Trios
                                                    unsigned int RawSegmentLength,  //  Byte length of RawSegment, for logging.
                                                    char *String,                   //  Translation from the SegmentCommand.ToStringCallback()
                                                    void *UserData);                //  Whatever you passed in as SegmentChangedUserData

/**
 *  Container struct to hold all static methods you need to 
 *  pass into the Segment Engine.
 */
typedef struct StaticConfiguration_t_{

    EndCallback_t End;
    void *EndUserData;

    TerminateCallback_t Terminate;
    void *TerminateUserData;

    SegmentChangeCallback_t SegmentChanged;
    void *SegmentChangedUserData;

}StaticCommands_t;

/** 
 *  Called to register your static commands with the Engine.
 */
int 
SeRegisterAllStaticCommands(ENGINE_HANDLE Engine, 
                            StaticCommands_t *StaticCommands
                            );


/** 
 *  Status that may be returned from some Segment API calls.
 */
enum SeStatus {

    SeSuccess,                      /**< Completed ok. */
    SeContinue,                     /**< Need more time, keep running this. */
    SeError,                        /**< Segment failed, end the experiment. */
    SeNotImplemented,               /**< Placeholder, no code yet */

    //
    //  You should have called SeSetIndexForBranch() before returning this.
    //  Your segment needs to have set ScfRepeat.
    //
    SeActiveSegmentIndexModified,   /**< The Segment modified the ActiveSegmentIndex - you need to have called SeSetIndexForBranch() */
    
    SeCommandIgnored,               /**< Command was not appropriate for engine state */
    SeInvalidParameters,            /**< Parameters passed in were invalid. */
    SeOutOfMemory,                  /**< Memory allocation failed */
    SeUnknownSegment,               /**< Segment passed in was not identified */
    SeUnsupportedSegmentType,       /**< The engine does not understand the type */
    SeInvalid,                      /**< The Segment declared the data associated with it invalid */
    SeDupSequenceNumber,            /**< Duplicate Sequence Numbers are not allowed in a list */
    SeDeletedCurrentSegment,        /**< Modifying the list actually deleted the active segment */

    //
    //  You must return one of these from your Start() callback if you are an abort conditional.
    //  Your segment needs to have set ScfAbortConditional.
    //
    SeAbortIfConditionAttach,       /**< Attach this segment to the next as a conditional */
    SeAbortIfConditionSkip,         /**< Skip the next segment, the condition is false to begin with. */

    //
    //  You must return one of these from your Run() callback if you are an abort conditional.
    //  Your segment needs to have set ScfAbortConditional.
    //
    SeAbortIfConditionTrue,         /**< An abort segment is true, skip the associated segment */
    SeAbortIfConditionFalse,        /**< An abort segment is false, run the associated segment */
};


/**
 *  Called exactly once to start a Segment.
 */
typedef SeStatus 
( *StartCallback_t)(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                unsigned char *Data, 
                                void *UserData, 
                                void *Extension
                                );

/**
 *  Optionally called once to stop a running a Segment, if that 
 *  segment was in progress.
 */
typedef void 
( *StopCallback_t)(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                unsigned char *Data, 
                                void *UserData, 
                                void *Extension
                                );

/**
 *  Optionally called multiple times to monitor and drive the 
 *  execution of a segment.
 */
typedef SeStatus 
( *RunCallback_t)( SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                            unsigned char *Data, 
                            void *UserData, 
                            void *Extension
                            );

/**
 *  Optionally called multiple times to modify the parameters of a 
 *  running segment.
 */
typedef void 
( *ModifyCallback_t)(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                unsigned char *Data, 
                                void *UserData, 
                                void *Extension
                                );

/**
 *  Called multiple times to validate a segment.  
 *  SeSuccess = valid 
 */
typedef SeStatus
( *ValidateCallback_t)(SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                unsigned char *Data, 
                                void *UserData
                                );

/**
 *  Called multiple times to fill in a buffer for a loggable string 
 *  representing the segment.
 */
typedef void 
( *ToStringCallback_t)(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                    unsigned char *Data, 
                                    void *UserData, 
                                    void *Extension,
                                    char *StringBuffer, 
                                    int *MaxStringBufferLength
                                    );


/**
 *  If this callback exists, it will be called "exactly once" right before 
 *  an experiment is started, i.e. the first segment is run.  This is an 
 *  opportunity to allocate data on a "per segment" basis.  If you don't 
 *  need Extension Data, just return NULL.
 */
typedef void *
( *SegmentSetupCallback_t)(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                        unsigned char *Data, 
                                        void *UserData
                                        );

/**
 *  If you have a SegmentSetupCallback_t, you "should" have this too.  
 *  (But it's not required)
 *  This will be called "exactly once" at the end of an experiment, 
 *  for all Segments whether they ran or not.  It's a chance to 
 *  clean up any allocations made in SegmentSetupCallback_t, via 
 *  the Extension pointer.
 */
typedef void 
( *SegmentCleanupCallback_t)(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                        unsigned char *Data, 
                                        void *UserData, 
                                        void *Extension);

/**
 *  If this exists, it will be called whenever your segment Start(), or Run()
 *  callbacks are called.  If you have a Lock, you need an Unlock.
 */
typedef void 
( *LockCallback_t)(    SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                unsigned char *Data, 
                                void *UserData, 
                                void *Extension);

/**
 *  If this exists, it will be called whenever your segment Start(), or Run()
 *  callbacks are called.   If you have a Lock, you need an Unlock.
 */
typedef void 
( *UnlockCallback_t)(  SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                unsigned char *Data, 
                                void *UserData, 
                                void *Extension);

/**
 *  Should be NULL unless you are some sort of REPEAT segment and you 
 *  want to adjust the search index on a predicate search.  This allows you 
 *  to init any data you will use custom for the search.
 */
typedef void 
( *InitSearchCallback_t)(  SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                    unsigned char *Data, 
                                    void *UserData, 
                                    void *Extension);

/**
 *  Should be NULL unless you are some sort of REPEAT segment and you 
 *  want to adjust the search index on a predicate search.  This allows you
 *  to return a modified working index during that search.
 *  If you changed the working segment index, return 
 *  SeActiveSegmentIndexModified AND fill in AdjustedWorkingIndex to the new 
 *  index.  Otherwise return SeSuccess.
 */
typedef SeStatus 
( *ModifyIndexForSearchCallback_t)(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                            unsigned char *Data, 
                                            void *UserData, 
                                            void *Extension,
                                            int *AdjustedWorkingIndex);


/**
 *  Flags to describe / change behavior for a Segment.
 *  Set bits in (SegmentCommand_t)->SegmentFlags
 */
enum SegmentCommandFlags {

    ScfDefault          = 0x00000000,
    ScfRepeat           = 0x00000001,
    ScfAbortConditional = 0x00000002,

};



/**
 *  Container struct to hold all methods you need to define 
 *  to create a "Segment".
 */
typedef struct SegmentCommand_t_ {

    /**
     *  These two values "together" must be unique. 
     */
    unsigned int Id;                                        /**< From SegmentBits_t */
    /**/

    unsigned int SegmentFlags;                              /**< Engine specific flags - default = 0 */
    unsigned int UserFlags;                                 /**< Custom flags, can be set to anything - can use SeFindNextSegmentIfUserFlags() to find */
    unsigned int DataLength;                                /**< Expected data length to be passed in. */
    void *UserData;                                         /**< Up to you, maybe a (this *)?... */

    /**
     *  Callbacks
     */
    StartCallback_t Start;                                  /**< Must exist! */
    ToStringCallback_t ToString;                            /**< Must exist! */
    RunCallback_t Run;                                      /**< Can be NULL if the Segment is instant */
    StopCallback_t Stop;                                    /**< Can be NULL if the Segment is instant */
    ModifyCallback_t Modify;                                /**< Can be NULL - Must be NULL if ScfRepeat or ScfAbortConditional is set */
    ValidateCallback_t Validate;                            /**< Can be NULL */
    SegmentSetupCallback_t Setup;                           /**< Can be NULL */
    SegmentCleanupCallback_t Cleanup;                       /**< Can be NULL */
    LockCallback_t Lock;                                    /**< Can be NULL */
    UnlockCallback_t Unlock;                                /**< Can be NULL */
    InitSearchCallback_t InitSearch;                        /**< Can be NULL */
    ModifyIndexForSearchCallback_t ModifyIndexForSearch;    /**< Can be NULL */

}SegmentCommand_t;


/** 
 *  Called to register your "Segment" with the Engine.
 *  @todo - Switch from ABORT to Error return codes?
 */
bool 
SeRegisterSegmentCommand(   ENGINE_HANDLE Engine, 
                            SegmentCommand_t *SegmentCommand
                            );

/**
 *  This is a custom comparison operator.
 *  
 *  @param DesiredUserFlags - The bit pattern you want.
 *  @param UserFlags -  User flags from the Segment list, passed in from each 
 *                      segment (SegmentCommand_t->UserFlags).
 *  @returns    1 => true - the operation you defined matches 
 *                          DesiredUserFlags & UserFlags
 *              0 => false - DesiredUserFlags & UserFlags did not match.
 */
typedef int (*UserFlagPredicate)(unsigned int DesiredUserFlags, unsigned int UserFlags);


/**
 *  Returns SegmentId of next segment that the Predicate returns true from.
 */
int 
SeFindNextSegmentIfUserFlags(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                UserFlagPredicate Predicate,
                                unsigned int DesiredUserFlags, 
                                unsigned int *Id
                                );

int 
SeFindNextSegmentIfUserFlagsEqual(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                    unsigned int DesiredUserFlags,
                                    unsigned int *Id
                                    );

int 
SeFindNextSegmentIfUserFlagsNotEqual(   SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                        unsigned int DesiredUserFlags,
                                        unsigned int *Id
                                        );

int 
SeFindNextSegmentIfUserFlagsAnyBitSet(  SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                        unsigned int DesiredUserFlags,
                                        unsigned int *Id
                                        );

int 
SeFindNextSegmentIfUserFlagsAllBitsSet( SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                                        unsigned int DesiredUserFlags,
                                        unsigned int *Id
                                        );


/**
 *  After calling this in your Run or Start callback, you need to 
 *  return SeActiveSegmentIndexModified.
 */
void 
SeSetIndexForBranch(    SE_SEGMENT_LIST_HANDLE SegmentListHandle, 
                        int NewIndex
                        );

unsigned int 
SeGetTotaltNumSegments( SE_SEGMENT_LIST_HANDLE SegmentListHandle
                        );

int 
SeGetCurrentSegmentIndex(   SE_SEGMENT_LIST_HANDLE SegmentListHandle
                            );


/**
 *  @returns    A Handle to a processed, validated Segment List that can
 *              be used by the Segment Engine.
 */
SE_SEGMENT_LIST_HANDLE 
SeCreateSegmentList(    ENGINE_HANDLE hEngine,  /**< We need to know what Engine we are going to run this on */
                        unsigned char *Data,    /**< Binary buffer of the Comm format of a SegmentList */
                        int Length,             /**< Length of Data */
                        SeStatus *ErrorStatus,  /**< Status of Parse */
                        int *SegmentIndex       /**< Index of Segment error, if applicable */
                        );


void 
SeDestroySegmentList(SE_SEGMENT_LIST_HANDLE SegmentListHandle);


/**
*  You should never call this from a callback,
*	or the engine will deadlock.
*/
int 
SeStartSegmentList(SE_SEGMENT_LIST_HANDLE SegmentListHandle);


/**
 *	You are allowed to call this from the End callback (or terminate callback)
 *	you registered for a SegmentListHandle associated with the same engine 
 *	as you are currently running on.
 *
 *	This is "specifically" coded to allow you to reuse the same 
 *	engine between pretest, test, and post test conditions.
 */
void 
SeStartSegmentListFromEndCallback(SE_SEGMENT_LIST_HANDLE SegmentListHandle);


/**
 *  This will trigger the static EndCallback_t.
 */
void 
SeStopSegmentList(SE_SEGMENT_LIST_HANDLE SegmentListHandle);



/**
 *  This will trigger the static TerminateCallback_t.
 */
void 
SeTerminateSegmentList(SE_SEGMENT_LIST_HANDLE SegmentListHandle);


/**
 *  If successful, a handle is returned that will be the active segment list 
 *  after this call.  The original list is unmodified, except for no longer 
 *  running.  Someone (the client) will need to clean it up.
 *
 *  You should never call this from a callback on the same engine,
 *	or the engine will deadlock.
 */
SE_SEGMENT_LIST_HANDLE 
SeModifySegmentList(    SE_SEGMENT_LIST_HANDLE OriginalListHandle,  /**< The list we are modifying */
                        unsigned char *Data,                        /**< Binary buffer of the Comm format of a SegmentList */
                        int Length,                                 /**< Length of Data */
                        SeStatus *ErrorStatus,                      /**< Status of Parse */
                        int *SegmentIndex                           /**< Index of Segment error, if applicable */
                        );


/**
 *  You should never call this from a callback on the same engine,
 *	or the engine will deadlock.
 */
int 
SeGotoNextSegment(SE_SEGMENT_LIST_HANDLE SegmentListHandle);




/**
 *  Internal Instrument Use Only!
 *  
 *  This API allows you to create an empty Segment List.  This is the 
 *  first step in generating your own internal segment lists.
 */ 
SE_SEGMENT_LIST_HANDLE 
SeInternalCreateEmptySegmentList(   ENGINE_HANDLE hEngine,
                                    SeStatus *ErrorStatus
                                    );


/**
 *  Internal Instrument Use Only!
 *
 *  This API allows you to add a segment to a Segment List.  This is 
 *  the second step in generating your own internal segment lists.
 */
SeStatus  
SeInternalAppendSegment(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                        unsigned int Id,
                        unsigned int UniqueTag,
                        unsigned char *Data,
                        int DataLength
                        );


// 
//  This is in ms.
//
#define SEGMENT_ENGINE_THREAD_RUN_PERIOD    25 



#ifdef __cplusplus
    }
#endif


#endif
