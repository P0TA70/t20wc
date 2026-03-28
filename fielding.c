#include "globals.h"

void *fielding(void *param) {
  Player *player = (Player *)param;
  while (!innings_ended) { // to allow fielders to die after innings done!
    pthread_mutex_lock(&fielder_mutex);

    while (!ball_in_air && !innings_ended) { // putting this just in case but what if somehow
                          // match_over = 1 when this signal is broadcase?
                          // safety measure!
      pthread_cond_wait(&BALL_HIT, &fielder_mutex);
    }

    if (innings_ended) {
      pthread_mutex_unlock(&fielder_mutex);
      pthread_exit(0);
    }
    
    if (wicket_type == CAUGHT) {
      // stats here
      printf("%s caught the ball midair!\n", player->name);
    } else if (wicket_type == RUNOUT) {
      // stats here
      printf("%s caught the ball when the batsman was taking runs!\n", player->name);
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
