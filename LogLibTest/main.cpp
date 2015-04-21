#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "LogLib.h"


int main(void)
{
    printf("Test Program for LogLib dynamic library.\n\n");


    printf("Calling InitLog() - ");
    InitLog();
    printf("complete.\n");

    LogMessage("First Log Message");
    LogMessage("Second Log Message");


    for (int i = 0; i<100; i++){
        LogMessage("Rapid Fire Log Message with varargs %d %s %f", i, "A String", 12.345);
    }



    sleep(2);
    return 0;
}
