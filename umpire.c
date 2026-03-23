#include "globals.h"
#include "structs.h"
#include "fifo_sem.h"
#include <stdlib.h>

//All common globs:
pthread_t bowlers[20];
BowlingScore bowler_stats[20];

//Common between bowler and batter:
BallOutcome balls[BALL_BUF_SIZE];

int new_ball, curr_ball, num_ball;
pthread_mutex_t pitch;
pthread_cond_t c_ba, c_bo;

//Common between batter and fielder:
WicketType wicket_type;
int ball_in_air;
pthread_cond_t BALL_HIT;
pthread_mutex_t fielder_mutex;
pthread_cond_t fielder_done;
pthread_mutex_t fielder_done_mutex;

//FIELDING globs:
int match_over = 0;//used in umpire // im adding this so that the fielder threads can exit after the innings is over! issa global var!

//BATTING globs:
int new_batsman; //used in umpire
pthread_mutex_t nb_mutex;

fifo_sem crease;
sem_t active_end, passive_end;

int score = 0, wickets = 0;

int balls_in_over = 0;
int over_count = 0;

int number_balls = 0;

void *bowling(void *param);
void *fielding(void *param);
void *batting(void *param);

void umpire(Team *ba, Team *bo) {
  fifo_sem_init(&crease, 2);
  sem_init(&active_end, 0, 1);
  sem_init(&passive_end, 0, 1);
  pthread_mutex_init(&pitch, NULL);
  pthread_cond_init(&c_ba, NULL);
  pthread_cond_init(&c_bo, NULL);
  pthread_t bowler;

  pthread_cond_init(&BALL_HIT, NULL);
  pthread_mutex_init(&fielder_mutex, NULL);
  pthread_cond_init(&fielder_done,NULL);
  pthread_mutex_init(&fielder_done_mutex, NULL);
  score = 0;

  new_batsman = 0;
  match_over = 0;           // incase umpire is called for 2nd innings!
  pthread_mutex_init(&nb_mutex,NULL);

  if (pthread_create(&bowler, NULL, bowling, (void *)bo)) {
    printf("Bowler thread couldnt be created");
    exit(1);
  }

  for (int i = 0; i < 11; i++) {
    if (pthread_create(&((*ba).players[i].id), NULL, batting, NULL)) {
      printf("Batter thread couldnt be created%d", i);
      exit(1);
    }
  }

  for (int i = 0; i < 11; i++) {
    if (pthread_create(&(*bo).players[i].id, NULL, fielding, NULL)) {
      printf("Fielder thread couldnt be created%d", i);
      exit(1);
    }
  }

  for (int i = 0; i < 11; i++) {
    pthread_join(ba->players[i].id, NULL);
  }

  match_over = 1;                    // end of innings signal!
  pthread_cond_broadcast(&BALL_HIT); // DEATH TO ALL FIELDERS!

  for (int i = 0; i < 11; i++) {
    pthread_join(bo->players[i].id, NULL);
  }
  pthread_join(bowler, NULL);

  pthread_mutex_destroy(&pitch);
  pthread_cond_destroy(&c_bo);
  pthread_cond_destroy(&c_ba);
  sem_destroy(&active_end);
  sem_destroy(&passive_end);
}

Team *construct_team(int data /*dummy, team bool for now*/) {
  // pretend some  probability stuff assigns proper stats
  Team *team = malloc(sizeof(Team));
  team->team_score = 0;
  for (int i = 0; i < 11; i++) {
    team->players[i].team = 0;
    team->players[i].death_over_specialist = i % 2;
    team->players[i].bo_score.balls_delivered = 0;
    team->players[i].bo_score.runs_given = 0;
    team->players[i].bo_score.wickets_taken = 0;
    team->players[i].ba_score.runs_taken = 0;
    team->players[i].ba_score.balls_received = 0;
    team->players[i].ba_score.fours = 0;
    team->players[i].ba_score.sixes = 0;
  }
  return team;
}
