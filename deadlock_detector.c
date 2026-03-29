#include "fifo_sem.h"
#include "globals.h"
#include "structs.h"
#include <unistd.h>
#include <time.h>
#define DETECTOR_TIME_PERIOD 10

void* deadlock(void* param)
{
    printf("Deadlock thread Initialized\n");
    while(1)
    {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 50000;
        nanosleep(&ts, NULL);
        // usleep(DETECTOR_TIME_PERIOD);
        if(innings_ended==1) break;
        else printf("I am the issue - solve me");

        pthread_mutex_lock(&deadlock_runout_mutex);
        if(deadlock_runout==1)
        {
            deadlock_runout = 0;
            pthread_mutex_unlock(&deadlock_runout_mutex);
            sleep(0.1);
            sem_post(&passive_end);
            printf(" - I (detector) released batter ");

        }
        else 
            pthread_mutex_unlock(&deadlock_runout_mutex);

    }
    printf("am i causing deadlock??");
    pthread_exit(NULL);
}