#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>





int main(void)
{
    int rc;

    rc = Init("gpio.conf");
    if (rc == -1){
        printf("Failed Gpio Init!");
    }

    PrintGpioConfig();

    return 0;
}



