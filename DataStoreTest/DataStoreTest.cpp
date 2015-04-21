#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "DataStore.h"




char *TestData1 = "Hello World!";
char *TestData2 = "Hello World a bit longer!";
char *TestData3 = "Hi";



int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    //----------------------------
    DataStoreStatus Status;
    unsigned char *Data;
    int DataLength;
    //----------------------------

    printf("\n\nUnit Testing DataStore code\n");

    Status = DataStoreWriteData( "TestData1.dat", 
                        (unsigned char *)TestData1, 
                        strlen(TestData1) + 1, 
                        DwfOverwrite);

    printf("Wrote: %s  Write Status = %d\n", TestData1, Status);


    Status = DataStoreReadData( "TestData1.dat", 
                        &Data, 
                        &DataLength, 
                        0);

    printf("Read Status = %d\n", Status);
    if (Status == DsSuccess){
        printf("READ: %s\n", Data);
        DataStoreFreeData(Data);
    }



    //
    //  No Overwrite.
    //
    Status = DataStoreWriteData( "TestData1.dat", 
                        (unsigned char *)TestData2, 
                        strlen(TestData2) + 1, 
                        DwfNone);

    printf("Wrote: %s Write Status = %d\n", TestData2, Status);


    Status = DataStoreReadData( "TestData1.dat", 
                        &Data, 
                        &DataLength, 
                        0);

    printf("Read Status = %d\n", Status);
    if (Status == DsSuccess){
        printf("READ: %s\n", Data);
        DataStoreFreeData(Data);
    }


    Status = DataStoreWriteData( "TestData1.dat", 
                        (unsigned char *)TestData3, 
                        strlen(TestData3) + 1, 
                        DwfOverwrite);

    printf("Write Status = %d\n", Status);





    Status = DataStoreReadData( "TestData1.dat", 
                        &Data, 
                        &DataLength, 
                        0);

    printf("Read Status = %d\n", Status);
    if (Status == DsSuccess){
        printf("READ: %s\n", Data);
        DataStoreFreeData(Data);
    }

    Status = DataStoreReadData( "TestData2.dat", 
                        &Data, 
                        &DataLength, 
                        0);

    printf("Read Status = %d\n", Status);
    if (Status == DsSuccess){
        printf("READ: %s\n", Data);
        DataStoreFreeData(Data);
    }


    typedef struct MyStruct_t_ {
        int A;
        int B;
        int C;
    }MyStruct_t;


    MyStruct_t Defaults = {1, 2, 3};

    Status = DataStoreWriteData( "Data.dat", 
                        (unsigned char *)&Defaults, 
                        sizeof(MyStruct_t), 
                        DwfOverwrite);

    printf("Write Defaults Status = %d\n", Status);


    MyStruct_t *MyStruct;
    int LengthRead;

    // Reading Data
    Status = DataStoreReadData( "Data.dat", 
                               (unsigned char **)&MyStruct, 
                               &LengthRead, 
                               0);

    printf("A = %d  B = %d  C = %d\n", MyStruct->A, MyStruct->B, MyStruct->C);
 
    MyStruct->A = 10; 
    MyStruct->B = 11; 
    MyStruct->C = 12;

    // Saving Data
    Status = DataStoreWriteData( "Data.dat", 
                               (unsigned char *)MyStruct, 
                               sizeof(MyStruct_t), 
                               DwfOverwrite);

    DataStoreFreeData((unsigned char *)MyStruct);
    
    Status = DataStoreReadData( "Data.dat", 
                               (unsigned char **)&MyStruct, 
                               &LengthRead, 
                               0);

    printf("A = %d  B = %d  C = %d\n", MyStruct->A, MyStruct->B, MyStruct->C);
 

	return 0;
}


