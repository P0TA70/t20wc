#include "fifo_sem.h"
#include "globals.h"
#include "structs.h"
#include <unistd.h>

#define DETECTOR_TIME_PERIOD 10

void* deadlock(void* param)
{
    while(1)
    {
        usleep(DETECTOR_TIME_PERIOD);
        if(innings_ended==1) break;
        pthread_mutex_lock(&deadlock_runout_mutex);
        if(deadlock_runout==1)
        {
            deadlock_runout = 0;
            pthread_mutex_unlock(&deadlock_runout_mutex);
            
            sem_post(&passive_end);
            printf(" - I (detector) released batter ");

        }
        else 
            pthread_mutex_unlock(&deadlock_runout_mutex);

    }
}