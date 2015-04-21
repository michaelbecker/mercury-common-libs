#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "LogLib.h"
#include "TaWinAssert.h"

#include "ConsoleMemoryDump.h"


static int LoggingEnabled = 0; 


void __attribute__  ((visibility ("default")))
EnableConsoleMemoryDumpLogging(int Enable)
{
    LoggingEnabled = Enable;
}


#define BUFFER_SIZE (4096 * 3)


void __attribute__  ((visibility ("default")))
ConsoleMemoryDump(char *Title, unsigned char *Buffer, int Length)
{
    //----------------------------
    char Character[17];
    int CharIndex = 0;
    int FullLines;
    int PartialLineCount;
    int BytesWritten = 0;
    int Offset = 0;
    char *OutputBuffer;
    //----------------------------

    //
    //  Syntatic sugar, make it look nicer...
    //
#define SAFE_BUFFER_INCREMENT() \
    {                           \
    if (BytesWritten >= 0){     \
        Offset += BytesWritten; \
    }                           \
    else{                       \
        goto EXIT_BUFFER_FULL;  \
    }                           \
    }


    //
    //  If we don't do anything, just leave.
    //
    if(!LoggingEnabled){
        return;
    }

    //
    //  We do NOT free this if we pass it to LogLib. 
    //
    OutputBuffer = (char *)malloc(BUFFER_SIZE);
    ASSERT(OutputBuffer != NULL);

    FullLines = Length / 16;
    PartialLineCount = Length % 16;

    BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, "\r\n%s - %d Bytes\r\n", Title, Length);
    SAFE_BUFFER_INCREMENT();
    BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, "------------------------------------+------------------\r\n");
    SAFE_BUFFER_INCREMENT();
    
    for (int line=0; line<FullLines; line++){

        for (int i = 0; i<16; i++){

            BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, "%02x", *Buffer);
            SAFE_BUFFER_INCREMENT();
            if (isprint(*Buffer)){
                Character[CharIndex] = *Buffer;
            }
            else{
                Character[CharIndex] = '.';
            }

            if ((i == 3) || (i == 7) || (i == 11)){
                BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, " ");
                SAFE_BUFFER_INCREMENT();
            }

            CharIndex++;
            Buffer++;
        }

        Character[16] = '\0';
        CharIndex = 0;
        BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, " | %s\r\n", Character);
        SAFE_BUFFER_INCREMENT();
    }


    if (PartialLineCount){

        int i;
        
        for (i = 0; i<PartialLineCount; i++){

            BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, "%02x", *Buffer);
            SAFE_BUFFER_INCREMENT();
            if (isprint(*Buffer)){
                Character[CharIndex] = *Buffer;
            }
            else{
                Character[CharIndex] = '.';
            }
            CharIndex++;
            Buffer++;

            if ((i == 3) || (i == 7) || (i == 11)){
                BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, " ");
                SAFE_BUFFER_INCREMENT();
            }
        }

        for ( ; i<16; i++){
            BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, "  ");
            SAFE_BUFFER_INCREMENT();
            Character[CharIndex] = ' ';
            CharIndex++;

            if ((i == 3) || (i == 7) || (i == 11)){
                BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, " ");
                SAFE_BUFFER_INCREMENT();
            }
        }

        Character[16] = '\0';
        BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, " | %s\r\n", Character);
        SAFE_BUFFER_INCREMENT();
    }
    BytesWritten = snprintf(OutputBuffer + Offset, BUFFER_SIZE, "------------------------------------+------------------\r\n");
    SAFE_BUFFER_INCREMENT();

    //
    //  Make sure this is null terminated.
    //
EXIT_BUFFER_FULL:
    OutputBuffer[BUFFER_SIZE - 1] = 0;

    LogMessageBuffer(OutputBuffer, Offset);
    // 
    //  Don't free OutputBuffer, LogLib will.
    //
}



