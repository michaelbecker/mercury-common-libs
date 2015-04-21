#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "MakeGuid.h"


/**
 *  Generates a UUID, version 4 number:
 *  xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
 *
 *  Version 4 UUIDs have the form xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx 
 *  where x is any hexadecimal digit and y is one of 8, 9, A, or B 
 *  (e.g., f47ac10b-58cc-4372-a567-0e02b2c3d479).
 *
 *  http://en.wikipedia.org/wiki/Universally_unique_identifier#Version_4_.28random.29
 */
static void
GenerateV4Guid(GUID *guid)
{
    //-------------------------------
    int d;
    //-------------------------------


    srand(time(NULL));

    d = rand() % 0xFFFF;
    guid->Data1 = (d << 16);
    d = rand() % 0xFFFF;
    guid->Data1 |= d;

    d = rand();
    guid->Data2[0] = d & 0xFF;
    d = rand();
    guid->Data2[1] = d & 0xFF;

    d = rand();
    guid->Data3[0] = (d & 0x0F) | 0x40;
    d = rand();
    guid->Data3[1] = d & 0xFF;
    
    d = rand();
    guid->Data4[0] = (d & 0x0F) | 0x80;
    d = rand();
    guid->Data4[1] = d & 0xFF;
    d = rand();
    guid->Data4[2] = d & 0xFF;
    d = rand();
    guid->Data4[3] = d & 0xFF;
    d = rand();
    guid->Data4[4] = d & 0xFF;
    d = rand();
    guid->Data4[5] = d & 0xFF;
    d = rand();
    guid->Data4[6] = d & 0xFF;
    d = rand();
    guid->Data4[7] = d & 0xFF;
}



static int
GetByteFromUuidString(  char *Buffer, 
                        int *index, 
                        int BufferLength)
{
    //----------------------------
    unsigned char Nibble1;
    unsigned char Nibble2;
    //----------------------------

    while(!isxdigit(Buffer[*index])){
        *index = *index + 1;
        if (*index >= BufferLength){
            return -1;
        }
    }
    
    Nibble1 = Buffer[*index];
    *index = *index + 1;
    if (*index >= BufferLength){
        return -1;
    }

    while(!isxdigit(Buffer[*index])){
        *index = *index + 1;
        if (*index >= BufferLength){
            return -1;
        }
    }
    
    Nibble2 = Buffer[*index];
    *index = *index + 1;
    if (*index >= BufferLength){
        return -1;
    }

    if (isdigit(Nibble1)){
        Nibble1 -= '0';
    }
    else if (isupper(Nibble1)){
        Nibble1 = Nibble1 - 'A' + 10;
    }
    else{
        Nibble1 = Nibble1 - 'a' + 10;
    }

    if (isdigit(Nibble2)){
        Nibble2 -= '0';
    }
    else if (isupper(Nibble2)){
        Nibble2 = Nibble2 - 'A' + 10;
    }
    else{
        Nibble2 = Nibble2 - 'a' + 10;
    }

    return (Nibble1 << 4) | Nibble2;
}



//  
//  Format: e4c7e328-936a-468d-b54d-e2e280e37a02
//
static int 
GenerateLinuxGuid(GUID *guid)
{
    //-------------------------------
    FILE *fp;
    char Buffer[38];
    int NumRead;
    int index;
    int Byte;
    int i;
    //-------------------------------

    fp = fopen("/proc/sys/kernel/random/uuid", "r");
    if (!fp){
        return -1;
    }

    NumRead = fread(Buffer, 1, sizeof(Buffer), fp);

    fclose(fp);

    if (NumRead < 36){
        return -1;
    }

    printf("%s", Buffer);
    index = 0;

    Byte = GetByteFromUuidString(Buffer, &index, NumRead);
    if (Byte < 0){
        return -1;
    }
    guid->Data1 = (Byte << 24);

    Byte = GetByteFromUuidString(Buffer, &index, NumRead);
    if (Byte < 0){
        return -1;
    }
    guid->Data1 |= (Byte << 16);

    Byte = GetByteFromUuidString(Buffer, &index, NumRead);
    if (Byte < 0){
        return -1;
    }
    guid->Data1 |= (Byte << 8);

    Byte = GetByteFromUuidString(Buffer, &index, NumRead);
    if (Byte < 0){
        return -1;
    }
    guid->Data1 |= (Byte << 0);


    Byte = GetByteFromUuidString(Buffer, &index, NumRead);
    if (Byte < 0){
        return -1;
    }
    guid->Data2[0] = Byte;

    Byte = GetByteFromUuidString(Buffer, &index, NumRead);
    if (Byte < 0){
        return -1;
    }
    guid->Data2[1] = Byte;
    
    Byte = GetByteFromUuidString(Buffer, &index, NumRead);
    if (Byte < 0){
        return -1;
    }
    guid->Data3[0] = Byte;

    Byte = GetByteFromUuidString(Buffer, &index, NumRead);
    if (Byte < 0){
        return -1;
    }
    guid->Data3[1] = Byte;


    guid->Data4[0] = 0;
    for (i = 0; i< 8; i++){
        Byte = GetByteFromUuidString(Buffer, &index, NumRead);
        if (Byte < 0){
            return -1;
        }
        guid->Data4[i] |= Byte;
    }

    return 0;
}



void 
GenerateGuid(GUID *guid)
{
    int rc = GenerateLinuxGuid(guid);
    if (rc < 0){
        GenerateV4Guid(guid);
    }
}



