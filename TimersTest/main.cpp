#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>


#include "Timers.h"



void Timer1 (void *UserData)
{
    printf("Timer1 - %lu\n", (unsigned long)UserData);
}




int main(void)
{
    TIMER_HANDLE h1, h2, h3, h4, h5;
    int rc;

    printf("Timers Unit Test\n\n");

    InitTimersLib();

    h1 = QueueTimerCallback(Timer1, (void *)1000, 1000);
    h2 = QueueTimerCallback(Timer1, (void *)2000, 2000);
    h3 = QueueTimerCallback(Timer1, (void *)3000, 3000);
    h4 = QueueTimerCallback(Timer1, (void *)4000, 4000);
    h5 = QueueTimerCallback(Timer1, (void *)5000, 5000);

    sleep(1);

    rc = CancelTimerCallback(h1);
    if (rc == 1){
        printf("Succeeding canceling H1? - might not be an error\n");
    }

    rc = CancelTimerCallback(h2);
    if (rc == 0){
        printf("Failed canceling H2? - might not be an error\n");
    }

    sleep(7);

    rc = CancelTimerCallback(h3);
    if (rc != 0){
        printf("Succeeding canceling H3?\n");
    }
    rc = CancelTimerCallback(h4);
    if (rc != 0){
        printf("Succeeding canceling H4?\n");
    }
    rc = CancelTimerCallback(h5);
    if (rc != 0){
        printf("Succeeding canceling H5?\n");
    }


    printf("Stress Test\n");

    TIMER_HANDLE stress[100];

    for (int i = 0; i<100; i++){
        stress[i] = QueueTimerCallback(Timer1, (void *)i, 990+i);
    }


    sleep(1);

    for (int i = 0; i<100; i++){

        if ((i % 2) == 0){

            rc = CancelTimerCallback(stress[i]);
            if (rc){
                printf("Cancelled %d\n", i);
            }
            else{
                printf("Missed Canceling %d\n", i);
            }
        }
    }
    

    sleep(5);


    return 0;
}




