#include "globals.h"
#include<unistd.h>


#define DETECTOR_INTERVAL 20000

void* deadlock(void* param)
{
    (void)param;

    while(1)
    {
        usleep(DETECTOR_INTERVAL);

        if(match_over) break;

        pthread_mutex_lock(&runout_mutex);
        if(runout_occuring)
        {
        

            printf("\nDeadlock D3TECT3D\n\n");

            pthread_mutex_lock(&nb_mutex);
            new_batsman = 1;
            pthread_mutex_unlock(&nb_mutex);
            
            runout_occuring = 0;

            sem_post(&passive_end); // Kick out active batsman
            fifo_sem_post(&crease);   // Allow next batsman to enter after removing active one

            printf("Deadlock R3COV3RED\n\n");

        }
        pthread_mutex_unlock(&runout_mutex);

    }
}