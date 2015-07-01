#define _CRT_SECURE_NO_WARNINGS
#include "ISegmentRepeatUntil.h"
#include "TaWinAssert.h"
#include "Version.h"



ISegmentRepeatUntil::ISegmentRepeatUntil(   ENGINE_HANDLE hEngine,
                                            unsigned int subCommand,
                                            unsigned int userFlags,
                                            unsigned int dataLength,
                                            char *name
                                            )
    :   Engine(hEngine),
        SubCommand(subCommand),
        UserFlags(userFlags),
        DataLength(dataLength),
        NewIndex(-1),
        DoBranch(false),
        Name(name)
{
    SegmentCommand_t Command = {

        SubCommand,             //  From CiiMessageData.<Type>.SubCommand     

        ScfRepeat,              //  SegmentFlags
        UserFlags,              //  Custom flags    
        DataLength,             //  Expected data length to be passed in.    
        this,                   //  Up to you    

        RouteStart,             //  Start - Must exist!     
        RouteToString,          //  ToString - Must exist!     
        NULL,                   //  Run - Can be NULL if the Segment is instant    
        NULL,                   //  Stop - Can be NULL if the Segment is instant    
        NULL,                   //  Modify - Can be NULL     
        RouteValidate,          //  Validate - Can be NULL     
        NULL,                   //  Setup - Can be NULL     
        NULL,                   //  Cleanup - Can be NULL    
        NULL,                   //  Lock - Can be NULL    
        NULL,                   //  Unlock - Can be NULL        
        NULL,                   //  InitSearch
        NULL,                   //  ModifyIndexForSearch
    };

    SeRegisterSegmentCommand(hEngine, &Command);

    LogVersion();
}



//  http ://msdn.microsoft.com/en-us/library/8183zf3x.aspx
ISegmentRepeatUntil::~ISegmentRepeatUntil()
{
}



void 
ISegmentRepeatUntil::BranchNeeded(int index)
{
    NewIndex = index;
    DoBranch = true;
}



void
ISegmentRepeatUntil::BranchNotNeeded()
{
    DoBranch = false;
}



SeStatus 
ISegmentRepeatUntil::RouteStart(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                unsigned char *Data,
                                void *UserData,
                                void *Extension)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentRepeatUntil *s = (ISegmentRepeatUntil *)UserData;

    s->StartCallback(Data);

    if (s->DoBranch){

        //
        //  Sanity, clear this out.
        //
        s->DoBranch = false;

        //
        //  Change the index of the segment list.
        //
        SeSetIndexForBranch(SegmentListHandle, s->NewIndex);

        //
        //  And let the engine know we did it.
        //
        return SeActiveSegmentIndexModified;
    }
    else{
        //
        //  We are done, move on to the next segment.
        //
        return SeSuccess;
    }
}



SeStatus 
ISegmentRepeatUntil::RouteValidate( SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                    unsigned char *Data,
                                    void *UserData)
{
    (void)SegmentListHandle;

    ISegmentRepeatUntil *s = (ISegmentRepeatUntil *)UserData;

    return s->ValidateCallback(Data);
}



void 
ISegmentRepeatUntil::RouteToString( SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                    unsigned char *Data,
                                    void *UserData,
                                    void *Extension,
                                    char *StringBuffer,
                                    int *MaxStringBufferLength)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentRepeatUntil *s = (ISegmentRepeatUntil *)UserData;

    s->ToStringCallback(Data, StringBuffer, MaxStringBufferLength);
}



unsigned int 
ISegmentRepeatUntil::GetDataLength()
{
    return DataLength;
}



const char *
ISegmentRepeatUntil::GetName()
{
    return Name;
}
