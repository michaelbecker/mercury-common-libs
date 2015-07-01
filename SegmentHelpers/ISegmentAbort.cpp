#include "ISegmentAbort.h"
#include "TaWinAssert.h"
#include "Version.h"


ISegmentAbort::ISegmentAbort(   ENGINE_HANDLE hEngine,
                                unsigned int subCommand,
                                unsigned int userFlags,
                                unsigned int dataLength,
                                char *name
                                )
    :   Engine(hEngine),
        SubCommand(subCommand),
        UserFlags(userFlags),
        DataLength(dataLength),
        Name(name)
{
    SegmentCommand_t Command = {

        SubCommand,             //  From CiiMessageData.<Type>.SubCommand     

        ScfAbortConditional,    //  SegmentFlags
        UserFlags,              //  Custom flags    
        DataLength,             //  Expected data length to be passed in.    
        this,                   //  Up to you    

        RouteStart,             //  Start - Must exist!     
        RouteToString,          //  ToString - Must exist!     
        RouteRun,               //  Run - Can be NULL if the Segment is instant    
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
ISegmentAbort::~ISegmentAbort()
{
}



SeStatus 
ISegmentAbort::RouteStart(  SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                            unsigned char *Data,
                            void *UserData,
                            void *Extension)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentAbort *s = (ISegmentAbort *)UserData;

    bool ShouldStart = s->StartNextSegment(Data);

    if (ShouldStart){
        return SeAbortIfConditionAttach;
    }
    else{
        return SeAbortIfConditionSkip;
    }
}



SeStatus 
ISegmentAbort::RouteRun(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                        unsigned char *Data,
                        void *UserData,
                        void *Extension)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentAbort *s = (ISegmentAbort *)UserData;

    bool ContinueSegment = s->ContinueNextSegment(Data);

    if (ContinueSegment){
        return SeAbortIfConditionFalse;
    }
    else{
        return SeAbortIfConditionTrue;
    }
}



SeStatus 
ISegmentAbort::RouteValidate(   SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                unsigned char *Data,
                                void *UserData)
{
    (void)SegmentListHandle;

    ISegmentAbort *s = (ISegmentAbort *)UserData;

    return s->ValidateCallback(Data);
}



void 
ISegmentAbort::RouteToString(   SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                unsigned char *Data,
                                void *UserData,
                                void *Extension,
                                char *StringBuffer,
                                int *MaxStringBufferLength)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentAbort *s = (ISegmentAbort *)UserData;

    s->ToStringCallback(Data, StringBuffer, MaxStringBufferLength);
}



unsigned int 
ISegmentAbort::GetDataLength()
{
    return DataLength;
}



const char *
ISegmentAbort::GetName()
{
    return Name;
}
