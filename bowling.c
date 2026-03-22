#include "globals.h"
#include <stdlib.h>

void *bowling(void *param) {
  Team *team = (Team *)param;

  for (int over = 0; over < 20; over++) {
    for (int i = 0; i < 6; i++) {
      int typeOfBall = rand() % 3;
      BallOutcome ball = typeOfBall;
      if (ball == NO_BALL || ball == WIDE)
        i--; // TODO:TUX IN DANGER
      pthread_mutex_lock(&pitch);
      if (num_ball > BALL_BUF_SIZE) {
        printf("crashing");
        fflush(stdout);
        exit(1);
      }
      while (num_ball == BALL_BUF_SIZE)
        pthread_cond_wait(&c_bo, &pitch);
      balls[new_ball] = ball;
      new_ball = (new_ball + 1) % BALL_BUF_SIZE;
      num_ball++;
      pthread_mutex_unlock(&pitch);
      pthread_cond_signal(&c_ba);
    }
  }
  return NULL;
}

