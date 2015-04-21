#ifndef COMM_FAULT_INJECTION_H__
#define COMM_FAULT_INJECTION_H__

#ifdef __cplusplus
extern "C" {
#endif


enum CommFaultInjectionOpcodes
{
    OpcodeCorruptSync,      // Corrupt all SYNC headers
    OpcodeChangeLengthByte, // All lengths +/-
    OpcodeChangeLength,     // Adds garbage or truncates data
    OpcodeNakDelay,         // Adds delays to all NAK messages
    OpcodeAckDelay,         // Adds delays to all ACK messages

    //OpcodeStatusStress,   // Sends Status messages very fast - implemented in the callback file.

    //
    //  TODO ...
    //
    OpcodeAckAll,
    OpcodeNakAll,
    OpcodeAckExtraData,      
    OpcodeAckDuplicates,
    OpcodeAckNak,
    OpcodeAckInvalidSeq,
    OpcodeNakNoData,      
    OpcodeNakExtraData,      
    OpcodeNakDuplicates,
    OpcodeNakAck,
    OpcodeNakInvalidSeq,
    OpcodeRspNoData,
    OpcodeRspInvalidSeq,
    OpcodeRspDuplicates,
    OpcodeRspThenAck,
    OpcodeRspThenNak,
    OpcodeStatusEmpty,
};


void  
CommFaultInjection(int Opcode, int Data1, int Data2);


#ifdef __cplusplus
    }
#endif


#endif
