/**
 *  Application interface to the Common Instrument Interface Library.
 */
#ifndef COMMON_INSTRUMENT_INTERFACE_H__
#define COMMON_INSTRUMENT_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "CommonInstrumentInterfaceBits.h"

/**
 *  This is an opaque handle assigned per message.
 */
typedef void * CII_MESSAGE_HANDLE;


/**
 *  Helper function.  An application will call this to add Response 
 *  data to a message in preperation to send it.
 *  
 *  YOU CAN ONLY CALL THIS ONCE!  MULTIPLE CALLS ON THE SAME 
 *  MESSAGE WILL ASSERT.
 */ 
void 
CiiAddOptionalResponseData( CII_MESSAGE_HANDLE Message, 
                            unsigned char *Data, 
                            unsigned int DataLength
                            );


/**
 *  If the app returns a MsPending, then it _must_ call this later.
 */ 
void 
CiiCompletePendingCommand(  CII_MESSAGE_HANDLE Message, 
                            MsgStatus Status
                            );


/**
 *  The callback structure.
 */
typedef MsgStatus (*MessageCallback_t)(CII_MESSAGE_HANDLE Message, void *UserData);


/**
 *  Allows the app to register a GET command message.
 *  This may be done at any time.  There is no way to remove a message
 *	once it's been registered.
 */
void 
CiiRegisterGetCommandMessage(   unsigned int Command, 
                                MessageCallback_t Callback,
                                void *UserData
                                );


/**
 *  Allows the app to register an ACTION command message.
 *  This may be done at any time.  There is no way to remove a message
 *	once it's been registered.
 */
void 
CiiRegisterActionCommandMessage(    unsigned int Command, 
                                    MessageCallback_t Callback, 
                                    void *UserData
                                    );


/**
 *  Allows the app to flag a message as no longer supported.
 */
void 
CiiDeprecateGetCommandMessage(  unsigned int Command
                                );

/**
 *  Allows the app to flag a message as no longer supported.
 */
void 
CiiDeprecateActionCommandMessage(   unsigned int Command
                                    );


/**
 *  Need to call to set up the library.
 */
void  
CiiInitializeLibrary(void);


/**
 *  Need to call before touching anything at all.
 */
void
CiiPreInitLibrary(void);


/**
 *  The app calls this to send a status message.
 */
void 
CiiSendStatus(  unsigned int SubStatus, 
                unsigned char *Data, 
                unsigned int DataLength
                );



void 
CiiGetCommandData(  CII_MESSAGE_HANDLE Msg,     //  [in]  Handle to the message.
                    unsigned char **Data,       //  [out] Pointer to the data pointer.
                    unsigned int *DataLength    //  [out] Pointer to the data size.
                    );



/**
 *  API to get the currently connected Master.
 *  If you pass in NULL as Info, only the status will be returned to you.
 */
int 
CiiGetActiveMaster(LoginInfo_t * Info);


/**
 *  API to get whether a LocalUI is connected or not.
 *  This returns the count of the Local UIs attached.
 */
int __attribute__  ((visibility ("default")))
CiiLocalUIActive(void);


/**
 *  NULL = no connections
 *  Otherwise, you need to free the memory yourself.
 */
DbgClientConnection_t * 
CiiDbgGetClientList(unsigned int *Count);


/**
 *  Force a close.
 */
void 
CiiDbgDropClient(unsigned int Handle);


/**
 *  Enable or disable tracing all data on all sockets.
 */
void 
CiiDbgEnableSocketTrace(int Enable);



#ifdef __cplusplus
    }
#endif


#endif
