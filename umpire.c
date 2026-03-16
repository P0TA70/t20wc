#include "globals.h"
#include "structs.h"
#include "fifo_sem.h"
#include <stdlib.h>

// globals h externs

fifo_sem crease;
sem_t active_end, passive_end;

int balls_in_over = 0;
int over_count = 0;

pthread_cond_t BALL_HIT;

// score for a team
int score = 0, wickets = 0;

// order of bowlers
pthread_t bowlers[20];
BowlingScore bowler_stats[20];

BallOutcome balls[BALL_BUF_SIZE];

int new_ball, curr_ball, num_ball;
pthread_mutex_t pitch;
pthread_cond_t c_ba, c_bo;
int number_balls = 0;

void *bowling(void *param);
void *fielding(void *param);
void *batting(void *param);

void *batting(void *param) {
  fifo_sem_wait(&crease);
  
  while (1) {
    sem_wait(&passive_end);
    sem_wait(&active_end);
    sem_post(&passive_end);

    // critical section
    while (1) {
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

      // deciding how batsman hit the ball
      if (balls[curr_ball] == LEGAL_BALL) {
        balls_in_over++;
        // fair ball
        int notOut = rand() % 15;
        if (notOut == 0) {
          WicketType typeOfWicket = rand() % 7;
          // TODO: add more cases here
          if (typeOfWicket == CAUGHT) {
            pthread_cond_broadcast(&BALL_HIT);
          }
          bowler_stats[over_count].balls_delivered++;
          bowler_stats[over_count].wickets_taken++;
          fifo_sem_post(&crease);
          sem_post(&active_end);
          pthread_exit(NULL);
        } else {
          int boundary = rand() % 9;
          if (boundary < 2) {
            BoundaryType bt = rand() % 2;
            if (bt == FOUR) {
              score += 4;
            } else {
              score += 6;
            }
          } else {
            int score_increase = rand() % 4;
          }
        }
      } else if (balls[curr_ball] == WIDE) {
        // wide ball
      } else {
        // no ball
      }

      if (balls_in_over == 6) {
        balls_in_over = 0;
        over_count++;
      }
      curr_ball = (curr_ball + 1) % BALL_BUF_SIZE;
      num_ball--;
      number_balls++;
      pthread_mutex_unlock(&pitch);
      pthread_cond_signal(&c_bo);
    }
    sem_post(&active_end);
  }
  fifo_sem_post(&crease);
  return NULL;
}

void *bowling(void *param) {
  Team *team = (Team *)param;

  // create a bowler list for 20 overs
  // TODO: select bowlers

  for (int over = 0; over < 20; over++) {
    // every over, the probabilities are reloaded
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

void *fielding(void *param) { return NULL; }

void umpire(Team *ba, Team *bo) {
  // initialise variables which will allow proper sequencing of the batsmen
  // then create threads

  fifo_sem_init(&crease, 2);
  sem_init(&active_end, 0, 1);
  sem_init(&passive_end, 0, 1);
  pthread_mutex_init(&pitch, NULL);
  pthread_cond_init(&c_ba, NULL);
  pthread_cond_init(&c_bo, NULL);
  pthread_t bowler;

  pthread_cond_init(&BALL_HIT, NULL);
  score = 0;

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

