#include "globals.h"

void *fielding(void *param) {
  Player *player = (Player *)param;
  while (!innings_ended) { 
    pthread_mutex_lock(&fielder_mutex);

    while (!ball_in_air && !innings_ended) { 
      pthread_cond_wait(&BALL_HIT, &fielder_mutex);
    }

    if (innings_ended) {
      pthread_mutex_unlock(&fielder_mutex);
      pthread_exit(0);
    }
    
    if (wicket_type == CAUGHT) {
      printf("Caught by %s midair", player->name);
    } else if (wicket_type == RUNOUT) {
      printf("Caught by %s when the batsman was taking runs!", player->name);
    }
    // do work, then signal

    pthread_mutex_lock(&fielder_done_mutex);
    ball_in_air = 0;
    pthread_mutex_unlock(&fielder_done_mutex);
    pthread_cond_signal(&fielder_done);

    pthread_mutex_unlock(&fielder_mutex);
  }
  return NULL;
}
