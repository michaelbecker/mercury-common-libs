/////////////////////////////////////////////////////////////////////////////
//
//                          DEBUGGING API
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CALCULATOR_DBG__H__
#define __CALCULATOR_DBG__H__


#ifdef __cplusplus
extern "C" {
#endif


void 
SmPrintFullSignalList(void);

void
SmPrintResults(int index);

void
SmPrintHeaders(void);

void 
SmStartSignalTrace(const char *Filename);

void 
SmStopSignalTrace(void);


enum SmDebugFlags{

    SmDbgApi                    = 0x00000001,
    SmDbgCalcLogic              = 0x00000002,
    SmDbgInputMgr               = 0x00000004,
    SmDbgDataSyncedCallbacks    = 0x00000008

};


void 
InitSmPrintf(void);


void 
SetSmDebugLevel(unsigned int level);



#ifdef __cplusplus
    }
#endif


#endif

