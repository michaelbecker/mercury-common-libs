#ifndef CONSOLE_MEMORY_DUMP_H__
#define CONSOLE_MEMORY_DUMP_H__

#ifdef __cplusplus
extern "C" {
#endif


void 
EnableConsoleMemoryDumpLogging(int Enable);

void 
ConsoleMemoryDump(char *Title, unsigned char *Buffer, int Length);


#ifdef __cplusplus
}
#endif



#endif
