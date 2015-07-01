#define _CRT_SECURE_NO_WARNINGS
#include "ISegmentInstant.h"
#include "TaWinAssert.h"
#include "Version.h"



ISegmentInstant::ISegmentInstant(   ENGINE_HANDLE hEngine,
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
ISegmentInstant::~ISegmentInstant()
{
}



unsigned int 
ISegmentInstant::GetSubcommand()
{
    return SubCommand;
}



SeStatus 
ISegmentInstant::RouteStart( SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                            unsigned char *Data,
                            void *UserData,
                            void *Extension)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentInstant *s = (ISegmentInstant *)UserData;

    return s->StartCallback(Data);
}



SeStatus 
ISegmentInstant::RouteValidate( SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                unsigned char *Data,
                                void *UserData)
{
    (void)SegmentListHandle;

    ISegmentInstant *s = (ISegmentInstant *)UserData;

    return s->ValidateCallback(Data);
}



void 
ISegmentInstant::RouteToString(  SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                                unsigned char *Data,
                                void *UserData,
                                void *Extension,
                                char *StringBuffer,
                                int *MaxStringBufferLength)
{
    (void)SegmentListHandle;
    (void)Extension;

    ISegmentInstant *s = (ISegmentInstant *)UserData;

    s->ToStringCallback(Data, StringBuffer, MaxStringBufferLength);
}



unsigned int 
ISegmentInstant::GetDataLength()
{
    return DataLength;
}



const char *
ISegmentInstant::GetName()
{
    return Name;
}
