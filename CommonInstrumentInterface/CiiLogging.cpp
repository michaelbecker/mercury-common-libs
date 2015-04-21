#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <map>
#include <list>

#include "TaWinAssert.h"
#include "CommonInstrumentInterface.h"
#include "CiiBackend.h"
#include "CommFaultInjectionPrivate.h"
#include "LogLib.h"
#include "WorkQueue.h"
#include "CiiPrivate.h"



static const char *
TranslateAccessLevel(AccessLevel access)
{
    switch (access){
        case AlInvalid:     return "AlInvalid";
        case AlViewOnly:    return "AlViewOnly";
        case AlMaster:      return "AlMaster";
        case AlLocalUI:     return "AlLocalUI";
        case AlEngineering: return "AlEngineering";
        default:            return "UNKNOWN";
    };
}



void
LogLogin(CiiClientRegistration_t *Client)
{
    //----------------------
    LoginInfo_t Info;
    //----------------------

    memcpy(&Info, &Client->Login.Info, sizeof(Info));

    Info.MachineName[63] = 0;
    Info.UserName[63] = 0;

    LogMessage("CiiLogin: Connect - %s / %s [%d.%d.%d.%d]", 
                Info.UserName,
                Info.MachineName,
                (int)Client->Login.Info.ClientIP[0],
                (int)Client->Login.Info.ClientIP[1],
                (int)Client->Login.Info.ClientIP[2],
                (int)Client->Login.Info.ClientIP[3]
                );

    LogMessage("CiiLogin: Access - Requested %s - Granted %s", 
        TranslateAccessLevel(Client->Login.RequestedAccess),
        TranslateAccessLevel(Client->GrantedAccess));
}



void
LogDisconnect(CiiClientRegistration_t *Client)
{
    //----------------------
    LoginInfo_t Info;
    //----------------------

    memcpy(&Info, &Client->Login.Info, sizeof(Info));

    Info.MachineName[63] = 0;
    Info.UserName[63] = 0;

    LogMessage("CiiLogin: Disconnect - %s / %s [%d.%d.%d.%d]", 
                Info.UserName,
                Info.MachineName,
                (int)Client->Login.Info.ClientIP[0],
                (int)Client->Login.Info.ClientIP[1],
                (int)Client->Login.Info.ClientIP[2],
                (int)Client->Login.Info.ClientIP[3]
                );

    LogMessage("CiiLogin: Access was: %s", 
        TranslateAccessLevel(Client->GrantedAccess));
}


