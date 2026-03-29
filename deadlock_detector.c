#include "fifo_sem.h"
#include "globals.h"
#include "structs.h"
#include <unistd.h>
#include <time.h>
#define DETECTOR_TIME_PERIOD 10

void* deadlock(void* param)
{
    while(1)
    {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 50000;
        nanosleep(&ts, NULL);
        if(innings_ended==1) break;

        pthread_mutex_lock(&deadlock_runout_mutex);
        if(deadlock_runout==1)
        {
            deadlock_runout = 0;
            pthread_mutex_unlock(&deadlock_runout_mutex);
            sleep(0.1);
            sem_post(&passive_end);
        }
        else 
            pthread_mutex_unlock(&deadlock_runout_mutex);

    }
    pthread_exit(NULL);
}