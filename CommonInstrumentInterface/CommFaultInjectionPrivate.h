#ifndef COMM_FAULT_INJECTION_PRIVATE_H__
#define COMM_FAULT_INJECTION_PRIVATE_H__

void CfiGetExtraRandomData(unsigned char *Buffer, int Amount);

bool CfiCorruptSync(void);

bool CfiCorruptLengthField(void);
int CfiCorruptLengthFieldValue(void);

int CfiCorruptLength(void);

extern int CfiNakDelay;
extern int CfiAckDelay;


#endif
