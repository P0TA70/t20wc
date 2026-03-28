#include "globals.h"
#include "structs.h"
#include <stdlib.h>

void *bowling(void *param) {
  Team *team = (Team *)param;

  for (int over = 0; over < 20; over++) {
    //assume some stats are updated here based on bowlers[over]
    int legal = team->players[bowlers[over]].pdf.legal;
    int wide = team->players[bowlers[over]].pdf.wide;

    for (int i = 0; i < 6; i++) {
      int typeOfBall = (rand() % 10000)+1;
      BallOutcome ball = (typeOfBall > legal + wide) ? NO_BALL : (typeOfBall > legal) ? WIDE : LEGAL_BALL;
      if (ball == NO_BALL || ball == WIDE)
        i--; 
      pthread_mutex_lock(&pitch);
      if (num_ball > BALL_BUF_SIZE) {
        printf("crashing");
        fflush(stdout);
        exit(1);
      }
      while (num_ball == BALL_BUF_SIZE && !innings_ended)
        pthread_cond_wait(&c_bo, &pitch);

      if (innings_ended) {
        pthread_exit(0);
      }
      
      balls[new_ball] = ball;
      new_ball = (new_ball + 1) % BALL_BUF_SIZE;
      num_ball++;
     
      pthread_mutex_unlock(&pitch);
      pthread_cond_signal(&c_ba);
    }
  }
  return NULL;
}

