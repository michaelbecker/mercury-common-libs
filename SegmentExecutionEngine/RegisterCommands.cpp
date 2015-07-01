#include <string.h>
#include <map>
#include "TaWinAssert.h"
#include "SegmentExecutionEngine.h"
#include "SegmentExecutionEnginePrivate.h"

using namespace std;



int __attribute__  ((visibility ("default")))
SeRegisterAllStaticCommands(ENGINE_HANDLE hEngine, 
                            StaticCommands_t *Config)
{
    //----------------------
    Engine_t *Engine;
    //----------------------

    ASSERT(hEngine != NULL);
    Engine = (Engine_t *)hEngine;
    ASSERT(Engine->Signature == ENGINE_SIGNATURE);
    ASSERT(Config != NULL);

    memcpy(&Engine->StaticCommandTable, Config, sizeof(StaticCommands_t));

    return true;
}



/**
 *  @todo - Switch from ABORT to Error return codes?
 */ 
bool __attribute__  ((visibility ("default")))
SeRegisterSegmentCommand(   ENGINE_HANDLE hEngine, 
                            SegmentCommand_t *UserSegmentConfig)
{
    //--------------------------------
    Engine_t *Engine;
    SegmentCommand_t *SegmentCommand;
    //--------------------------------

    ASSERT(hEngine != NULL);
    Engine = (Engine_t *)hEngine;
    ASSERT(Engine->Signature == ENGINE_SIGNATURE);

    ASSERT(UserSegmentConfig);
    ASSERT(UserSegmentConfig->Start != NULL);
    ASSERT(UserSegmentConfig->ToString != NULL);

    if (UserSegmentConfig->Lock){
        ASSERT(UserSegmentConfig->Unlock);
    }
    if (UserSegmentConfig->Unlock){
        ASSERT(UserSegmentConfig->Lock);
    }

    //
    //  Repeats are instant, Modify makes no sense.
    //
    if (UserSegmentConfig->SegmentFlags & ScfRepeat){
        ASSERT(UserSegmentConfig->Modify == NULL);
    }
    //
    //  Aborts are "kind of" instant, Modify also makes no sense.
    //
    if (UserSegmentConfig->SegmentFlags & ScfAbortConditional){
        ASSERT(UserSegmentConfig->Modify == NULL);
    }


    SegmentCommand = (SegmentCommand_t *)malloc(sizeof(SegmentCommand_t));
    ASSERT(SegmentCommand);

    memcpy(SegmentCommand, UserSegmentConfig, sizeof(SegmentCommand_t));

    (*Engine->SegmentCommandTable)[SegmentCommand->Id] = SegmentCommand;

    return true;
}

