//---------------------------------------------------------------------------
//  Example 1 
//  Sends:      ACK + Default Positive Response 
//                          (subcommand + Status code = MsSuccess)
//  Executes:   All within the callback
//---------------------------------------------------------------------------
static MsgStatus __stdcall
CallbackFoo(CII_MESSAGE_HANDLE Message, void *UserData)
{
    //  Do stuff...

    return MsSuccess;
}


//---------------------------------------------------------------------------
//  Example 2
//  Sends:      ACK + Positive Response with Data (subcommand 
//                              + Status code = MsSuccess + <Optional Data>)
//  Executes:   All within the callback
//---------------------------------------------------------------------------
static MsgStatus __stdcall
CallbackFoo(CII_MESSAGE_HANDLE Message, void *UserData)
{
    //  Do stuff...

    //  Add optional Data
    CiiAddOptionalResponseData( Message, MyData, MyDataLength);

    //  Do more stuff...

    return MsSuccess;
}


//---------------------------------------------------------------------------
//  Example 3
//  Sends:      NAK Code
//  Executes:   All within the callback
//---------------------------------------------------------------------------
static MsgStatus __stdcall
CallbackFoo(CII_MESSAGE_HANDLE Message, void *UserData)
{
    //  Do stuff...

    //  Return any int > 0 to denote failure, MsFailed = 1
    //  This code will be returned to Trios in the NAK.
    return MsFailed;
}


//---------------------------------------------------------------------------
//  Example 4
//  Returns:    ACK + Default Positive Response 
//                      (subcommand + Status code = MsSuccess)
//  Executes:   Delayed Completion
//---------------------------------------------------------------------------
static MsgStatus __stdcall
CallbackFoo(CII_MESSAGE_HANDLE Message, void *UserData)
{
    //  Do stuff...

    //  Save the Message Handle
    GlobalCallbackFooMessageHandle = Message;

    //  Do more stuff...

    return MsPending;
}

void SomeOtherThread()
{
    // ...

    //  In another thread later on...
    CiiCompletePendingCommand(GlobalCallbackFooMessageHandle, MsSuccess);

    // ...
}


//---------------------------------------------------------------------------
//  Example 5
//  Returns:    ACK + Default Negative Response 
//                      (subcommand + Status code = ERROR)
//  Executes:   Delayed Completion
//---------------------------------------------------------------------------
static MsgStatus __stdcall
CallbackFoo(CII_MESSAGE_HANDLE Message, void *UserData)
{
    //  Do stuff...

    //  Save the Message Handle
    GlobalCallbackFooMessageHandle = Message;

    //  Do more stuff...

    return MsPending;
}

void SomeOtherThread()
{
    // ...

    //  In another thread later on...
    //  Return int > 0 to denote failure, MsFailed = 1
    CiiCompletePendingCommand(GlobalCallbackFooMessageHandle, MsSFailed);

    // ...
}


//---------------------------------------------------------------------------
//  Example 6
//  Sends:      ACK + Positive Response with Data (subcommand 
//                              + Status code = MsSuccess + <Optional Data>)
//  Executes:   Delayed Completion
//---------------------------------------------------------------------------
static MsgStatus __stdcall
CallbackFoo(CII_MESSAGE_HANDLE Message, void *UserData)
{
    //  Do stuff...

    //  Save the Message Handle
    GlobalCallbackFooMessageHandle = Message;

    //  Do more stuff...

    return MsPending;
}

void SomeOtherThread()
{
    // ...

    //  Add optional Data, prior to calling CiiCompletePendingCommand()
    CiiAddOptionalResponseData( GlobalCallbackFooMessageHandle, 
                                MyData, 
                                MyDataLength);

    //  In another thread later on...
    CiiCompletePendingCommand(GlobalCallbackFooMessageHandle, MsSuccess);

    // ...
}


//---------------------------------------------------------------------------
//  Example 7
//  Sends:      ACK + Negative Response with Data (subcommand 
//                              + Status code = ERROR + <Optional Data>)
//  Executes:   Delayed Completion
//---------------------------------------------------------------------------
static MsgStatus __stdcall
CallbackFoo(CII_MESSAGE_HANDLE Message, void *UserData)
{
    //  Do stuff...

    //  Save the Message Handle
    GlobalCallbackFooMessageHandle = Message;

    //  Do more stuff...

    return MsPending;
}

void SomeOtherThread()
{
    // ...

    //  Add optional Data, prior to calling CiiCompletePendingCommand()
    CiiAddOptionalResponseData( GlobalCallbackFooMessageHandle, 
                                MyData, 
                                MyDataLength);

    //  In another thread later on...
    //  Return int > 0 to denote failure, MsFailed = 1
    CiiCompletePendingCommand(GlobalCallbackFooMessageHandle, MsSFailed);

    // ...
}


//---------------------------------------------------------------------------
//  Example 8
//  Returns:    ACK + Default Negative Response 
//                      (subcommand + Status code = ERROR)
//  Executes:   All within the callback
//---------------------------------------------------------------------------
static MsgStatus __stdcall
CallbackFoo(CII_MESSAGE_HANDLE Message, void *UserData)
{
    //  Do stuff...

    //  Return int > 0 to denote failure, MsFailed = 1
    CiiCompletePendingCommand(Message, MsSFailed);

    //  Do more stuff...

    //  Return anything actually, it will be ignored. 
    //  CiiCompletePendingCommand() already sent an ACK + the 
    //  negative default response.
    return MsSuccess;
}


//---------------------------------------------------------------------------
//  Example 9
//  Returns:    ACK + Negative Response with Data (subcommand 
//                              + Status code = ERROR + <Optional Data>)
//  Executes:   All within the callback
//---------------------------------------------------------------------------
static MsgStatus __stdcall
CallbackFoo(CII_MESSAGE_HANDLE Message, void *UserData)
{
    //  Do stuff...

    //  Add optional Data - Must call prior to CiiCompletePendingCommand()
    CiiAddOptionalResponseData( Message, MyData, MyDataLength);

    //  Return int > 0 to denote failure, MsFailed = 1
    CiiCompletePendingCommand(Message, MsSFailed);

    //  Do even more stuff...

    //  Return anything actually, it will be ignored. 
    //  CiiCompletePendingCommand() already sent an ACK + the 
    //  negative response.
    return MsSuccess;
}

