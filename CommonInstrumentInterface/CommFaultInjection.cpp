#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>

#include "TaWinAssert.h"
#include "CommonInstrumentInterface.h"
#include "CiiBackend.h"
#include "CommFaultInjection.h"
#include "CommFaultInjectionPrivate.h"


using namespace std;


///////////////////////////////////////////////////////////////////
static bool CorruptSync = false;

bool CfiCorruptSync(void)
{
    return CorruptSync;
}


///////////////////////////////////////////////////////////////////
static int CorruptLengthField = 0;

bool CfiCorruptLengthField(void)
{
    if (CorruptLengthField)
        return true;
    else
        return false;
}

int CfiCorruptLengthFieldValue(void)
{
    return CorruptLengthField;
}


///////////////////////////////////////////////////////////////////
static int CorruptLength = 0;

int CfiCorruptLength(void)
{
    return CorruptLength;
}


///////////////////////////////////////////////////////////////////
int CfiNakDelay = 0;
int CfiAckDelay = 0;


///////////////////////////////////////////////////////////////////
void __attribute__  ((visibility ("default")))
CommFaultInjection(int Opcode, int Data1, int Data2)
{
    switch (Opcode){

        case OpcodeCorruptSync:

            if (Data1){
                CorruptSync = true;
            }
            else{
                CorruptSync = false;
            }
            break;

        case OpcodeChangeLengthByte:
            CorruptLengthField = Data1;
            break;

        case OpcodeChangeLength:
            CorruptLength = Data1;
            break;

        case OpcodeNakDelay:
            CfiNakDelay = Data1;
            break;

        case OpcodeAckDelay:
            CfiAckDelay = Data1;
            break;

        case OpcodeAckAll:
            break;

        case OpcodeNakAll:
            break;

        case OpcodeAckExtraData:
            break;

        case OpcodeAckDuplicates:
            break;

        case OpcodeAckNak:
            break;

        case OpcodeAckInvalidSeq:
            break;

        case OpcodeNakNoData:
            break;

        case OpcodeNakExtraData:
            break;

        case OpcodeNakDuplicates:
            break;

        case OpcodeNakAck:
            break;

        case OpcodeNakInvalidSeq:
            break;

        case OpcodeRspNoData:
            break;

        case OpcodeRspInvalidSeq:
            break;

        case OpcodeRspDuplicates:
            break;

        case OpcodeRspThenAck:
            break;

        case OpcodeRspThenNak:
            break;

        case OpcodeStatusEmpty:
            break;

        default:
            break;
    }
}


void 
CfiGetExtraRandomData(unsigned char *Buffer, int Amount)
{
    for (int i = 0; i<Amount; i++){
        *Buffer++ = (rand() % 0x100);
    }
}

