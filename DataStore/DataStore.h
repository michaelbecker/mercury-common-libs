/*
 *  Example Usage:
 *  =============================
 *  MyStruct_t *MyStruct;
 *  DWORD LengthRead;
 *
 *  // Reading Data
 *  DataStoreStatus Status = DataStoreReadData( _T("Data.dat"), 
 *                              (unsigned char **)&MyStruct, 
 *                              &LengthRead, 
 *                              0);
 *
 *  // Saving Data
 *  DataStoreStatus Status = DataStoreWriteData( _T("Data.dat"), 
 *                              (unsigned char *)MyStruct, 
 *                              sizeof(MyStruct_t), 
 *                              DwfOverwrite);
 *
 */
#ifndef DATA_STORE_H__
#define DATA_STORE_H__

#ifdef __cplusplus
extern "C" {
#endif


enum DataStoreStatus {

    DsSuccess,              // ReadData/WriteData:  All is good
    DsBadInput,             // ReadData/WriteData:  Function args are incorrect
    DsIoError,              // ReadData/WriteData:  IO error
    DsAlreadyExists,        // WriteData:           The data exists already and you said no overwrite
    DsDataDoesNotExist,     // ReadData:            The DataId does not exist
};


//
//  Passed into DataStoreWriteData() Flags parameter.
//
enum DataWriteFlags {

    DwfNone         = 0x0,  // Default behavior - no overwrite
    DwfOverwrite    = 0x1,  // Overwrite the file if it exists

};


//
//  Store the data with the Data Id tag given.
//
DataStoreStatus 
DataStoreWriteData( char * DataId, 
                    unsigned char *DataBuffer, 
                    int DataBufferLength, 
                    unsigned int Flags);

//
//  Retrieve the data from the Data Id Tag.
//  You are given a pointer to the data in AppDataBuffer.
//  You are given the length of this data in AppDataBufferLength.
//
//  If this function succeeds, you own the memory returned in AppDataBuffer.
//  Remember to free it using DataStoreFreeData() before reusing the 
//  pointer in another call to DataStoreReadData().  If this function 
//  fails, do NOT call  DataStoreFreeData().  
//
DataStoreStatus 
DataStoreReadData(  char * DataId,                  // In
                    unsigned char **AppDataBuffer,  // Out
                    int *AppDataBufferLength,     // Out
                    unsigned int Flags);                   // In

//
//  Call this when you need to discard an AppDataBuffer.
//  Repeated calls to DataStoreReadData() with the same AppDataBuffer
//  pointer will leak memory without a call to DataStoreFreeData().
//
void 
DataStoreFreeData(  unsigned char *AppDataBuffer);



//
//  Call this to delete all trace of the non-volatile data storage.
//
void 
DataStorePurgeData(char * DataId);  // In



void 
DataStoreInit(void);


#ifdef __cplusplus
}
#endif



#endif
