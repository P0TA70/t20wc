#include "fifo_sem.h"
#include <stdint.h>
#include "globals.h"
#include "structs.h"
#include <stdlib.h>

//visualiser function
void ganttChart(int* output,int size,Team* ba);

// All common globs:
int bowlers[20];
char *bowler_names[20];
int batter_chart[1024];

// Common between bowler and batter:
BallOutcome balls[BALL_BUF_SIZE];

int new_ball, curr_ball, num_ball;
pthread_mutex_t pitch;
pthread_cond_t c_ba, c_bo;

// Common between batter and fielder:
WicketType wicket_type;
int ball_in_air;
pthread_cond_t BALL_HIT;
pthread_mutex_t fielder_mutex;
pthread_cond_t fielder_done;
pthread_mutex_t fielder_done_mutex;

// BATTING globs:
int new_batsman; // used in umpire
pthread_mutex_t nb_mutex;

fifo_sem crease;
sem_t active_end, passive_end;

int score, wickets;

int balls_in_over;
int over_count;

int number_balls;

int innings_ended;

int deadlock_runout;
pthread_mutex_t deadlock_runout_mutex;

int old;

void *bowling(void *param);
void *fielding(void *param);
void *batting(void *param);
void *deadlock(void *param);

Results umpire(Team *ba, Team *bo, int sched, int old_score) {
  new_ball=0, curr_ball=0, num_ball=0, score=0, wickets=0, balls_in_over=0, over_count=0, number_balls=0, innings_ended=0, deadlock_runout=0;
  
  old = old_score;
  fifo_sem_init(&crease, 2);
  sem_init(&active_end, 0, 1);
  sem_init(&passive_end, 0, 1);
  pthread_mutex_init(&pitch, NULL);
  pthread_cond_init(&c_ba, NULL);
  pthread_cond_init(&c_bo, NULL);
  pthread_t bowler;

  pthread_cond_init(&BALL_HIT, NULL);
  pthread_mutex_init(&fielder_mutex, NULL);
  pthread_cond_init(&fielder_done, NULL);
  pthread_mutex_init(&fielder_done_mutex, NULL);
  score = 0;

  new_batsman = 0;
  pthread_mutex_init(&nb_mutex, NULL);

  // populate bowler order in ids here
  //  dummy
  int bowler_rankings[11];
  double bowler_perfs[11];
  int specialists[5] = {-1, -1, -1, -1, -1};
  int index = 0;

  for (int i = 0; i < 11; i++) {
    PDF *pdf = &bo->players[i].pdf;

    bowler_rankings[i] = i; // FOR NOW RANDOM
    bowler_perfs[i] = ((double)rand()) / RAND_MAX;
    if (pdf->death_over_specialist == 1 && index < 5) {
      specialists[index] = i;
      index++;
    }
  }
  // sort
  for (int i = 0; i < 11; i++) {
    for (int j = i; j >= 0 && (bowler_perfs[j - 1] > bowler_perfs[j]); j--) {
      double temp = bowler_perfs[j - 1];
      bowler_perfs[j - 1] = bowler_perfs[j];
      bowler_perfs[j] = temp;
      int t = bowler_rankings[j - 1];
      bowler_rankings[j - 1] = bowler_rankings[j];
      bowler_rankings[j] = t;
    }
  }
  for (int i = 0; i < 15; i++) {
    bowlers[i] = bowler_rankings[i % 11];
    bowler_names[i] = bo->players[bowlers[i]].name;
  }
  for (int i = 15; i < 15 + index; i++) {
    bowlers[i] = specialists[i - 15];
    bowler_names[i] = bo->players[bowlers[i]].name;
  }
  for (int i = 15 + index; i < 20; i++) {
    bowlers[i] = bowler_rankings[i % 11];
    bowler_names[i] = bo->players[bowlers[i]].name;
  }

  if (pthread_create(&bowler, NULL, bowling, (void *)bo)) {
    printf("Bowler thread couldnt be created");
    exit(1);
  }
  pthread_t deadlock_thread;
  if (pthread_create(&deadlock_thread, NULL, deadlock, NULL)) {
    printf("Deadlock thread couldnt be created");
    exit(1);
  }
  
  // populate batter order
  int batter_order[11];
  double batter_perfs[11];

  for (int i = 0; i < 11; i++) {
    PDF *pdf = &ba->players[i].pdf;
    batter_order[i] = i; 
    batter_perfs[i] = 10000-pdf->out;
  }
  for (int i = 0; i < 11; i++) {
    for (int j = i; j >= 0 && (batter_perfs[j - 1] > batter_perfs[j]); j--) {
      double temp = batter_perfs[j - 1];
      batter_perfs[j - 1] = batter_perfs[j];
      batter_perfs[j] = temp;
      int t = batter_order[j - 1];
      batter_order[j - 1] = batter_order[j];
      batter_order[j] = t;
    }
  }
  if (sched == 0) {
    //0 for fcfs, 1 for sjf
    for (int i = 0; i < 5; i++) {
      double temp = batter_perfs[i];
      batter_perfs[i] = batter_perfs[10 - i];
      batter_perfs[10 - i] = temp;
      int t = batter_order[i];
      batter_order[i] = batter_order[10 - i];
      batter_order[10 - i] = t;
    }
  }

  for (int i = 0; i < 11; i++) {
    if (pthread_create(&((*ba).players[batter_order[i]].tid), NULL, batting,
                       (void *)(&ba->players[batter_order[i]]))) {
      printf("Batter thread couldnt be created%d", i);
      exit(1);
    }
  }

  for (int i = 0; i < 11; i++) {
    if (pthread_create(&(*bo).players[i].tid, NULL, fielding,
                       (void *)(&bo->players[i]))) {
      printf("Fielder thread couldnt be created%d", i);
      exit(1);
    }
  }

  Results return_value;

  void* batter_wait_times[11];  

  for (int i = 0; i < 11; i++) {
    pthread_join(ba->players[batter_order[i]].tid, &batter_wait_times[i]);
    return_value.wait_balls[i] = (intptr_t)batter_wait_times[i];
    return_value.batter_names[i]=ba->players[batter_order[i]].name;
  }
  return_value.score = score;

  //visualiser function 
  ganttChart(batter_chart,number_balls,ba);

  pthread_mutex_lock(&fielder_mutex);
  pthread_mutex_lock(&pitch);
  innings_ended = 1;
  pthread_mutex_unlock(&pitch);
  pthread_mutex_unlock(&fielder_mutex);
  pthread_cond_broadcast(&BALL_HIT); // DEATH TO ALL FIELDERS!
  for (int i = 0; i < 11; i++) {
    pthread_join(bo->players[i].tid, NULL);
  }

  pthread_cond_signal(&c_bo);
  pthread_join(bowler, NULL);
  pthread_join(deadlock_thread,NULL);
  pthread_mutex_destroy(&pitch);
  pthread_cond_destroy(&c_bo);
  pthread_cond_destroy(&c_ba);
  sem_destroy(&active_end);
  sem_destroy(&passive_end);

  return return_value;
}
