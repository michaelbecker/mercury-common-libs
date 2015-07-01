#define _CRT_SECURE_NO_WARNINGS
#include "ISegmentSimple.h"
#include "TaWinAssert.h"
#include "Version.h"



ISegmentSimple::ISegmentSimple( ENGINE_HANDLE hEngine,
                                unsigned int subCommand,
                                unsigned int userFlags,
                                unsigned int dataLength,
                                char *name
                                )
    :Engine(hEngine),
    SubCommand(subCommand),
    UserFlags(userFlags),
    DataLength(dataLength),
    Name(name)
{
    SegmentCommand_t Command = {

        SubCommand,             //  From CiiMessageData.<Type>.SubCommand     

        ScfDefault,             //  SegmentFlags
        UserFlags,              //  Custom flags    
        DataLength,             //  Expected data length to be passed in.    
        this,                   //  Up to you    

        RouteStart,             //  Start - Must exist!     
        RouteToString,          //  ToString - Must exist!     
        RouteRun,               //  Run - Can be NULL if the Segment is instant    
        RouteStop,              //  Stop - Can be NULL if the Segment is instant    
        RouteModify,            //  Modify - Can be NULL     
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
ISegmentSimple::~ISegmentSimple()
{
}



unsigned int 
ISegmentSimple::GetSubcommand()
{
    return SubCommand;
}



SeStatus 
ISegmentSimple::RouteStart( SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                            unsigned char *Data,
                            void *UserData,
                            void *Extension)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentSimple *s = (ISegmentSimple *)UserData;

    return s->StartCallback(Data);
}



void 
ISegmentSimple::RouteStop(  SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                            unsigned char *Data,
                            void *UserData,
                            void *Extension)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentSimple *s = (ISegmentSimple *)UserData;

    s->StopCallback(Data);
}



SeStatus 
ISegmentSimple::RouteRun(   SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                            unsigned char *Data,
                            void *UserData,
                            void *Extension)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentSimple *s = (ISegmentSimple *)UserData;

    return s->RunCallback(Data);
}



void 
ISegmentSimple::RouteModify(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                            unsigned char *Data,
                            void *UserData,
                            void *Extension)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentSimple *s = (ISegmentSimple *)UserData;

    s->ModifyCallback(Data);
}



SeStatus 
ISegmentSimple::RouteValidate(  SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                unsigned char *Data,
                                void *UserData)
{
    (void)SegmentListHandle;

    ISegmentSimple *s = (ISegmentSimple *)UserData;

    return s->ValidateCallback(Data);
}



void 
ISegmentSimple::RouteToString(  SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                unsigned char *Data,
                                void *UserData,
                                void *Extension,
                                char *StringBuffer,
                                int *MaxStringBufferLength)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentSimple *s = (ISegmentSimple *)UserData;

    s->ToStringCallback(Data, StringBuffer, MaxStringBufferLength);
}



//
//  Default behavior is to do nothing.
//
void 
ISegmentSimple::StopCallback(unsigned char *Data)
{
    (void)Data;
    return;
}



//
//  Default behavior is to end the segment immediately.
//  If you don't override this, your start "should"
//  never let you get here.
//
SeStatus
ISegmentSimple::RunCallback(unsigned char *Data)
{
    (void)Data;
    return SeSuccess;
}



//
//  Default behavior is to do nothing.
//
void
ISegmentSimple::ModifyCallback(unsigned char *Data)
{
    (void)Data;
    return;
}



unsigned int 
ISegmentSimple::GetDataLength()
{
    return DataLength;
}



const char *
ISegmentSimple::GetName()
{
    return Name;
}
