#include "globals.h"

void *fielding(void *param) {
  while (!match_over) {                                   // to allow fielders to die after innings done!
    pthread_mutex_lock(&fielder_mutex); 

    while (!ball_in_air && !match_over) {                 // putting this just in case but what if somehow match_over = 1 
                                                          // when this signal is broadcase? safety measure!
      pthread_cond_wait(&BALL_HIT,&fielder_mutex);
    }

    if (match_over) {                                     // match(innings..im not changing it) over case!
      pthread_mutex_unlock(&fielder_mutex);               
      break;                                              
    }

    if (wicket_type==CAUGHT) {
      // stats here
    } else if (wicket_type==RUNOUT) {
      // stats here
    }
    //do work, then signal

    pthread_mutex_lock(&fielder_done_mutex);
    ball_in_air=0;
    pthread_mutex_unlock(&fielder_done_mutex);
    pthread_cond_signal(&fielder_done);

    pthread_mutex_unlock(&fielder_mutex);
  }
  return NULL;
}

