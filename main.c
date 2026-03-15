#include "globals.h"
#include "structs.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// globals h externs
sem_t crease;
sem_t active_end, passive_end;
sem_t is_bowling;
FILE *ballsInOver; 



BallOutcome ball_outcome;
pthread_mutex_t pitch_mut;
pthread_cond_t BALL_THROW;

Team *construct_team(int data /*dummy*/);
void umpire(Team *ba, Team *bo);

void *bowler(void *param);
void bowling();
void fielding();
void *batting(void *param);

int main() {
  srand(time(NULL));
  ballsInOver = tmpfile();
  if (ballsInOver == NULL) {
    perror("Failed to create temporary file");
    return 1;
  }  
  
  printf("here i am");
  // some input data is converted to 2 teams
  Team *a = construct_team(0);
  Team *b = construct_team(1);

  int coin = rand() % 2;
  if (coin) {
    umpire(a, b);
  } else {
    umpire(b, a);
  }

  return 0;
}

void *batting(void *param) {
  sem_wait(&crease);
  sem_wait(&passive_end);
  sem_wait(&active_end);
  sem_post(&passive_end);

  // critical section

  sem_post(&active_end);

  return NULL;
}

void bowling() {
  srand(time(0));
  for (int i=0;i<6;i++) {
    int typeOfBall = rand()%3;
    BallOutcome ball = typeOfBall;
    fprintf(ballsInOver, "%d", ball);  
    fprintf(ballsInOver,"\n");
  }
  rewind(ballsInOver); 
}

void fielding() {
  return;
}

void *bowler(void *param) {
  while (1) { //TODO: ADD A CONDITION 
    int check = sem_trywait(&is_bowling);
    if (check == 0) {
      bowling();
    } else if (check == -1) {
      fielding();
    } else {
      printf("Error! Exiting");
      exit(1);
    }
  }

  return NULL;
}

void umpire(Team *ba, Team *bo) {
  // initialise variables which will allow proper sequencing of the batsmen
  // then create threads

  sem_init(&crease, 0, 2);
  sem_init(&active_end, 0, 1);
  sem_init(&passive_end, 0, 1);
  sem_init(&is_bowling, 0, 1);

  for (int i = 0; i < 11; i++) {
    if (pthread_create(&((*ba).players[i].id), NULL, batting, NULL)) {
      printf("Batter thread couldnt be created%d", i);
      exit(1);
    }
  }

  for (int i = 0; i < 11; i++) {
    if (pthread_create(&(*bo).players[i].id, NULL, bowler, NULL)) {
      printf("Bowler thread couldnt be created%d", i);
      exit(1);
    }
  }

  for (int i = 0; i < 11; i++) {
    pthread_join(ba->players[i].id, NULL);
  }

  for (int i = 0; i < 11; i++) {
    pthread_join(bo->players[i].id, NULL);
  }

  sem_destroy(&crease);
  sem_destroy(&active_end);
  sem_destroy(&passive_end);
  sem_destroy(&is_bowling);
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
