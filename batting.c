#include "globals.h"
#include <stdlib.h>

int runout_occuring = 0; // deadlocker
pthread_mutex_t runout_mutex;

void *batting(void *param) {
  Player *player = (Player *)param;
  int boundary_p = player->pdf.boundary;
  int out_p = player->pdf.out;
  fifo_sem_wait(&crease);

  int got_out =
      0; // adding ts cuz when previously a batsman got out, crease was being
         // posted twice extra post was allowing 3rd batsman to come!

  while (1) { // this loop is so that the 2 current batsmen rotate
    pthread_mutex_lock(&nb_mutex);
    if (!new_batsman) {
      pthread_mutex_unlock(&nb_mutex);
      sem_wait(&passive_end);
      sem_wait(&active_end);
      sem_post(&passive_end);
    } else {
      new_batsman = 0;
      pthread_mutex_unlock(&nb_mutex);
    }

    // critical section
    int notOut =
        1; // wide and no ball case was leaving ts uninitialized, so i added it!
    int over_end = 0; // to check if the current over has ended!
    while (1) {       // this loop is for the 1 active batsman
      pthread_mutex_lock(&pitch);
      if (num_ball < 0) {
        printf("crash");
        fflush(stdout);
        exit(1);
      } /* underflow */
      while (num_ball == 0) /* block if buffer empty */
        pthread_cond_wait(&c_ba, &pitch);
      /* if executing here, buffer not empty so remove element */
      printf("%d #%lx %d \n", number_balls, pthread_self(), balls[curr_ball]);
      fflush(stdout);

      BallOutcome ball = balls[curr_ball];

      curr_ball = (curr_ball + 1) % BALL_BUF_SIZE; // !!!
      num_ball--;
      number_balls++;
      pthread_mutex_unlock(&pitch); // we were not releasing this earlier!
      pthread_cond_signal(&c_bo);

      // deciding how batsman hit the ball
      if (ball == LEGAL_BALL) {
        balls_in_over++;
        // fair ball
        notOut = (rand() % 10000 > out_p) ? 1 : 0;
        if (notOut == 0) {
          WicketType typeOfWicket = rand() % 7;
          // TODO: add more cases here
          if (typeOfWicket == CAUGHT || typeOfWicket == RUNOUT) {
            if (typeOfWicket == RUNOUT) {
              int runs = rand() % 4;
              score += runs;
            }
            if (typeOfWicket == CAUGHT) {
              bowler_stats[over_count].wickets_taken++;
            }
            pthread_mutex_lock(&fielder_mutex);
            ball_in_air = 1;
            wicket_type = typeOfWicket;
            pthread_mutex_unlock(&fielder_mutex);
            pthread_cond_signal(&BALL_HIT);

            pthread_mutex_lock(&fielder_done_mutex);
            while (ball_in_air) {
              pthread_cond_wait(&fielder_done, &fielder_done_mutex);
            }
            pthread_mutex_unlock(&fielder_done_mutex);

            if (typeOfWicket == RUNOUT) {
              bowler_stats[over_count].balls_delivered++;

              pthread_mutex_lock(&runout_mutex);
              runout_occuring = 1;
              pthread_mutex_unlock(&runout_mutex);

              printf("\nThis guy is Run out NOW , Oh detector please help\n");
              // D3ADLOCK3D
              sem_wait(&passive_end); // condition for RUN OUT

              got_out = 1;
              pthread_exit(NULL);
            }
          }

          bowler_stats[over_count].balls_delivered++;

          pthread_mutex_lock(&nb_mutex);
          new_batsman = 1;
          pthread_mutex_unlock(&nb_mutex);

          got_out =
              1; // this is to prevent double post of that crease semaphore!
          fifo_sem_post(&crease);
          if (!new_batsman) {
            sem_post(&active_end);
          } else {
            break;
          }
          pthread_exit(NULL);
        } else {
          int boundary = (rand() % 9999 + 1 < boundary) ? 1 : 0;
          if (boundary) {
            BoundaryType bt = rand() % 2;
            if (bt == FOUR) {
              score += 4;
            } else {
              score += 6;
            }
          } else {
            int score_increase = rand() % 4;
            score += score_increase;
            if (score_increase % 2 != 0) {
              break;
            }
          }
        }
      } else if (ball == WIDE) {
        // wide ball
        score += 1;
        int not_overthrow = rand() % 10;
        if (!not_overthrow) {
          // overthrow case here
          int notRunOut = (rand() % 10000 > out_p) ? 1 : 0;
          if (notRunOut == 0) {
            int runs = rand() % 4;
            score += runs;

            pthread_mutex_lock(&fielder_mutex);
            ball_in_air = 1;
            wicket_type = RUNOUT;
            pthread_mutex_unlock(&fielder_mutex);
            pthread_cond_signal(&BALL_HIT);

            pthread_mutex_lock(&fielder_done_mutex);
            while (ball_in_air) {
              pthread_cond_wait(&fielder_done, &fielder_done_mutex);
            }
            pthread_mutex_unlock(&fielder_done_mutex);

            bowler_stats[over_count].balls_delivered++;

            pthread_mutex_lock(&runout_mutex);
            runout_occuring = 1;
            pthread_mutex_unlock(&runout_mutex);

            printf("\nThis guy is Run out NOW , Oh detector please help\n");
            // D3ADLOCK3D
            sem_wait(&passive_end); // condition for RUN OUT

            got_out = 1;
            pthread_exit(NULL);
          } else {
            int runs = rand() % 5;
            score += runs;
            if (runs % 2 != 0) {
              break;
            }
          }
        }
      } else {
        // no ball
        score++;
        int hit = rand() % 2;
        if (hit) {
          int notOut = rand() % 2; // out will be only through run out
          if (notOut == 0) {
            int runs = rand() % 4;
            score += runs;
            // make wicket type as run out and just do what was done for fielder
          } else {
            int notBoundary = rand() % 6;
            if (notBoundary == 0) {
              BoundaryType what_boundary = rand() % 2;
              if (what_boundary == FOUR) {
                score += 4;
              } else {
                score += 6;
              }
            } else {
              int runs = rand() % 4;
              score += runs;
              if (runs % 2 != 0) {
                break;
              }
            }
          }
        }
        // freeHit();  // TODO
      }

      if (balls_in_over == 6) {
        balls_in_over = 0;
        over_count++;
        over_end = 1; // flag to signify over has ended!
      }
      if (over_end) {
        sem_post(&active_end);
        break; // swap sides of the pitch!
      }
    }
    if (notOut == 0) {
      break;
    }
    if (!over_end)
      sem_post(&active_end); // for odd runs side change!
  }
  if (!got_out) { // post crease only if not posted above!
    fifo_sem_post(&crease);
  }
  return NULL;
}
