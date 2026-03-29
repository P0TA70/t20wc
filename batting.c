#include "fifo_sem.h"
#include "globals.h"
#include "structs.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>

int new_over_bool = 1;

void *batting(void *param) {
  Player *player = (Player *)param;
  PDF *pdf = &player->pdf;
  int balls_received = 0;
  int return_value;

  fifo_sem_wait(&crease);
  while (1) {
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
    while (1) {
      if (balls_received == 0) {
        return_value = number_balls;
        balls_received = 1;
      }
      if (over_count == 20 || wickets == 10 || score > old) {
        sem_post(&active_end);
        fifo_sem_post(&crease);
        pthread_exit((void *)(intptr_t)(return_value));
      }

      // typical producer consumer
      pthread_mutex_lock(&pitch);
      if (num_ball < 0) {
        fprintf(stderr, "crash\n");
        exit(1);
      }

      while (num_ball == 0) {
        printf("\nim waiting here help\n");
        pthread_cond_wait(&c_ba, &pitch);
      }

      BallOutcome ball = balls[curr_ball];

      curr_ball = (curr_ball + 1) % BALL_BUF_SIZE;
      num_ball--;
      number_balls++;

      pthread_mutex_unlock(&pitch);
      pthread_cond_signal(&c_bo);
      // now the consumer has taken the resource (ball)

      // randomising values
      StrokeOutcome so;
      so.wicket_bool = ((rand() % 10000 + 1) < pdf->out);
      so.wicket_type = rand() % 5;
      so.boundary_bool = ((rand() % 10000 + 1) < pdf->boundary);
      so.boundary_type = rand() % 2;
      so.runs = (so.wicket_bool == 0 && so.boundary_bool == 1 && ball != WIDE)
                    ? ((so.boundary_type == FOUR) ? 4 : 6)
                    : rand() % 4;

      int overthrow_bool = 1;

      switch (ball) {
      case LEGAL_BALL:
        balls_in_over++;
        break;
      case WIDE:
        overthrow_bool = rand() % 2;
        break;
      default:
        break;
      }

      // score changing block
      if (ball == WIDE || ball == NO_BALL) {
        score++;
      }

      if (overthrow_bool == 1 &&
          ((so.wicket_bool == 0) ||
           (so.wicket_bool == 1 &&
            (ball != LEGAL_BALL || so.wicket_type == RUNOUT)))) {
        score += so.runs;
      }

      // printing block
      if (new_over_bool) {
        printf("\n Over %d - Bowler %s\n", over_count + 1,
               bowler_names[over_count]);
        fflush(stdout);
        new_over_bool = 0;
      }

      printf("[Over %d.%d] %s    ", over_count + 1, balls_in_over,
             player->name);
      printf("%s  ", (ball == LEGAL_BALL) ? "Legal"
                     : (ball == WIDE)     ? "Wide(+1 score!)"
                                          : "Noball(+1 score!)");
      if (so.wicket_bool)
        printf("%s", (so.wicket_type == RUNOUT)   ? "RUNOUT!  "
                     : (so.wicket_type == CAUGHT) ? "CAUGHT!  "
                                                  : "OUT!  ");
      else if (so.boundary_bool)
        printf("%s", (so.boundary_type == FOUR) ? "FOUR!  " : "SIX!  ");
      else
        printf("Hit  ");

      if ((so.wicket_bool && so.wicket_type == RUNOUT) ||
          (so.wicket_bool == 0 && so.boundary_bool == 0))
        printf("Ran %d runs  ", so.runs);

      printf("(%d/%d)  ", score, wickets+so.wicket_bool);

      fflush(stdout);

      // the fielder chain block
      if (so.wicket_bool == 1 &&
          (so.wicket_type == RUNOUT || so.wicket_type == CAUGHT)) {
        // tell fielder that  ball in air
        pthread_mutex_lock(&fielder_mutex);
        ball_in_air = 1;
        wicket_type = so.wicket_type;
        pthread_mutex_unlock(&fielder_mutex);
        pthread_cond_signal(&BALL_HIT);

        // wait for fielder to remove ball from air
        pthread_mutex_lock(&fielder_done_mutex);
        while (ball_in_air) {
          pthread_cond_wait(&fielder_done, &fielder_done_mutex);
        }
        pthread_mutex_unlock(&fielder_done_mutex);
      }
      printf("\n");
      fflush(stdout);
      // the over ender block

      if (balls_in_over == 6) {
        over_count++;
        balls_in_over = 0;
        so.runs = 1 - so.runs % 2;
        new_over_bool = 1;
      }

      // the kill yourself block

      if (so.wicket_bool == 0 && so.runs % 2 == 1) {

        int passive;
        sem_getvalue(&passive_end, &passive);
        while (crease.value != 0 || passive != 0) {
          sem_getvalue(&passive_end, &passive);
        };

        sem_post(&active_end);
        break;
      } else if (so.wicket_bool == 1 && so.runs % 2 == 0) {
        pthread_mutex_lock(&nb_mutex);
        new_batsman = 1;
        pthread_mutex_unlock(&nb_mutex);

        wickets++;

        if (wickets == 10) {
          sem_post(&active_end);
        }

        fifo_sem_post(&crease);
        pthread_exit((void *)(intptr_t)(return_value));
      } else if (so.wicket_bool == 1 && so.runs % 2 == 1) {
        int passive;
        sem_getvalue(&passive_end, &passive);
        while (crease.value != 0 || passive != 0) {
          sem_getvalue(&passive_end, &passive);
        };

        wickets++;
        sem_post(&active_end);
        fifo_sem_post(&crease);
        pthread_exit((void *)(intptr_t)(return_value));
      }
    }
  }
  return NULL;
}
