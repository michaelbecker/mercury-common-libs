#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <map>
#include "DataStore.h"
#include "RunTimeErrorsStub.h"
//#include "RunTimeErrors.h"
//#include "common_system_logs.h"
#include "CommonFiles.h"
#include "TaWinAssert.h"
#include "Version.h"


using namespace std;


//
//  Sanity check the size of a config file.
//
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_PATH 1024


static map<unsigned char *, int>MemoryTracking;
static pthread_mutex_t MemoryTrackingLock = PTHREAD_MUTEX_INITIALIZER;  // TENTATIVE - try this with static init. - mb
static volatile unsigned int AllocatedMemoryBuffers = 0;


void __attribute__  ((visibility ("default")))
DataStorePurgeData(char * DataId)
{
    //-----------------------------
    char FullFilename[MAX_PATH];
    int wcharsSaved;
    //-----------------------------

    wcharsSaved = snprintf(FullFilename, MAX_PATH, DATA_ROOT "%s", DataId);
    if ((wcharsSaved < 0) ||(wcharsSaved == MAX_PATH) ){
        return;
    }
    
    unlink(FullFilename);
}



DataStoreStatus __attribute__  ((visibility ("default")))
DataStoreWriteData( char * DataId, 
                    unsigned char *DataBuffer, 
                    int DataBufferLength, 
                    unsigned int Flags)
{
    //-----------------------------
    int hFile;
    char FullFilename[MAX_PATH];
    bool FileExists;
    int wcharsSaved;
    int NumberOfBytesWritten;
    //-----------------------------

    if (!DataId){
        return DsBadInput;
    }

    if (!DataBuffer){
        return DsBadInput;
    }

    if ((DataBufferLength == 0) || (DataBufferLength > MAX_FILE_SIZE)){
        return DsBadInput;
    }

    wcharsSaved = snprintf(FullFilename, MAX_PATH, DATA_ROOT "%s", DataId);
    if ((wcharsSaved < 0) ||(wcharsSaved == MAX_PATH) ){
        return DsBadInput;
    }

    //
    //  Determine if the file exists and how big it is.
    //
    
    struct stat st;
    int rc = stat(FullFilename, &st);

    if (rc != 0){
        FileExists = false;
    }
    else{
        FileExists = true;
    }

    //
    //  The file exists, use the overwrite flag to decide what to do.
    //
    if (FileExists){
        if (Flags & DwfOverwrite){
            hFile = creat(FullFilename, 0644);
        }
        else{
            return DsAlreadyExists;
        }
    }
    //
    //  Easy case file DNE, just make it.
    //
    else{
        hFile = creat(FullFilename, 0644);
    }

    if (hFile == -1){

        PulseError(  MajorCommonCortex_CreateFileFailed, 
                    (unsigned char *)&errno, 
                    sizeof(errno));

        return DsIoError;
    }


    NumberOfBytesWritten = write(hFile, DataBuffer, DataBufferLength);

    //
    //  Must wait to CloseHandle, so we don't loose the error code.
    //
    if (NumberOfBytesWritten != DataBufferLength){

        PulseError(  MajorCommonCortex_WriteFileFailed, 
                    (unsigned char *)&errno, 
                    sizeof(errno));

        close(hFile);
        return DsIoError;
    }
    else{
        close(hFile);
        return DsSuccess;
    }
}



DataStoreStatus __attribute__  ((visibility ("default")))
DataStoreReadData(  char * DataId,                  // In
                    unsigned char **AppDataBuffer,  // Out
                    int *AppDataBufferLength,     // Out
                    unsigned int Flags)                    // In
{
    //-----------------------------
    int hFile;
    char FullFilename[MAX_PATH];
    int wcharsSaved;
    int NumberOfBytesRead;
    unsigned char *DataBuffer;
    int DataBufferLength;
    //-----------------------------

    (void)Flags;

    if (!DataId){
        return DsBadInput;
    }

    if (!AppDataBuffer){
        return DsBadInput;
    }

    if (!AppDataBufferLength){
        return DsBadInput;
    }

    wcharsSaved = snprintf(FullFilename, MAX_PATH, DATA_ROOT "%s", DataId);
    if ((wcharsSaved < 0) ||(wcharsSaved == MAX_PATH) ){
        return DsBadInput;
    }

    hFile = open(FullFilename, O_RDONLY);

    if (hFile == -1){
        return DsDataDoesNotExist;
    }


    struct stat st;

    int rc = fstat(hFile, &st);

    if (rc != 0){
        int Error = errno;

        PulseError(  MajorCommonCortex_ReadFileFailed, 
                    (unsigned char *)&Error, 
                    sizeof(Error));

        close(hFile);
        return DsIoError;
    }

    DataBufferLength = st.st_size;
    
    DataBuffer = (unsigned char *)malloc(DataBufferLength);
    ASSERT(DataBuffer);


    NumberOfBytesRead = read(hFile, DataBuffer, DataBufferLength);

    if (NumberOfBytesRead != DataBufferLength){

        PulseError(  MajorCommonCortex_ReadFileFailed, 
                    (unsigned char *)&errno, 
                    sizeof(errno));

        close(hFile);
		memset(DataBuffer, 0xEE, DataBufferLength);
        free(DataBuffer);
        return DsIoError;
    }

    close(hFile);

    *AppDataBuffer = DataBuffer;
    *AppDataBufferLength = DataBufferLength;

	//
	//	Lock ----------------------------------------
	//
	pthread_mutex_lock(&MemoryTrackingLock);

	MemoryTracking[DataBuffer] = DataBufferLength;
	AllocatedMemoryBuffers++;

	//
	//	Unlock --------------------------------------
	//
	pthread_mutex_unlock(&MemoryTrackingLock);

    return DsSuccess;
}



void __attribute__  ((visibility ("default")))
DataStoreFreeData(unsigned char *AppDataBuffer)
{
    //------------------------------------------
	ASSERT(AppDataBuffer);
	map<unsigned char *, int>::iterator it;
	int Length;
    //------------------------------------------

	//
	//	Lock ----------------------------------------
	//
	pthread_mutex_lock(&MemoryTrackingLock);

	it = MemoryTracking.find(AppDataBuffer);
	ASSERT(it != MemoryTracking.end());

	Length = it->second;

	MemoryTracking.erase(it);

	AllocatedMemoryBuffers--;

	//
	//	Unlock --------------------------------------
	//
	pthread_mutex_unlock(&MemoryTrackingLock);

	memset(AppDataBuffer, 0xEE, Length);
    free(AppDataBuffer);
}



void __attribute__  ((visibility ("default")))
DataStoreInit(void)
{
    LOG_LIBRARY_VERSION();
}


