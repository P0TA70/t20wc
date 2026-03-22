#include "globals.h"
#include "structs.h"
#include "fifo_sem.h"
#include <stdlib.h>

// globals h externs
WicketType wicket_type;

int new_batsman;
pthread_mutex_t nb_mutex;

fifo_sem crease;
sem_t active_end, passive_end;

int balls_in_over = 0;
int over_count = 0;

int ball_in_air;
pthread_cond_t BALL_HIT;
pthread_mutex_t fielder_mutex;
pthread_cond_t fielder_done;
pthread_mutex_t fielder_done_mutex;

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
    pthread_mutex_lock(&nb_mutex);
    if (!new_batsman) {
      pthread_mutex_unlock(&nb_mutex);
      sem_wait(&passive_end);
      sem_wait(&active_end);
      sem_post(&passive_end);
    } else {
      new_batsman=0;
      pthread_mutex_unlock(&nb_mutex);
    }

    // critical section
    int notOut;
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
        notOut = rand() % 15;
        if (notOut == 0) {
          WicketType typeOfWicket = rand() % 7;
          // TODO: add more cases here
          if (typeOfWicket==CAUGHT) {
            pthread_mutex_lock(&fielder_mutex);
            ball_in_air=1;
            wicket_type=typeOfWicket; 
            pthread_mutex_unlock(&fielder_mutex);
            pthread_cond_signal(&BALL_HIT);
            
            pthread_mutex_lock(&fielder_done_mutex);
            while (ball_in_air) {
              pthread_cond_wait(&fielder_done,&fielder_done_mutex);
            } 
            pthread_mutex_unlock(&fielder_done_mutex);
          } else if (typeOfWicket==RUNOUT) {
            int runs = rand()%4;
            score+=runs;

            pthread_mutex_lock(&fielder_mutex);
            ball_in_air=1;
            wicket_type=typeOfWicket; 
            pthread_mutex_unlock(&fielder_mutex);
            pthread_cond_signal(&BALL_HIT);
            
            pthread_mutex_lock(&fielder_done_mutex);
            while (ball_in_air) {
              pthread_cond_wait(&fielder_done,&fielder_done_mutex);
            } 
            pthread_mutex_unlock(&fielder_done_mutex);
          } 
          
          bowler_stats[over_count].balls_delivered++;
          bowler_stats[over_count].wickets_taken++;

          pthread_mutex_lock(&nb_mutex);
          new_batsman = 1;
          pthread_mutex_unlock(&nb_mutex);

          fifo_sem_post(&crease);
          if (!new_batsman) {
            sem_post(&active_end);
          } else {
            break;
          }
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
            score+=score_increase;
            if (score_increase%2!=0) {
              break; 
            }
          }
        }
      } else if (balls[curr_ball] == WIDE) {
        // wide ball
        // team stats for run out (cause its not considered bowler's wicket)
        score+=1;
        int not_overthrow = rand()%10;
        if (!not_overthrow) {
          // overthrow case here 
          int notRunOut = rand()%4;
          if (notRunOut==0) {
            int runs = rand()%4;
            score+=runs;

            pthread_mutex_lock(&fielder_mutex);
            ball_in_air=1;
            wicket_type=RUNOUT; 
            pthread_mutex_unlock(&fielder_mutex);
            pthread_cond_signal(&BALL_HIT);
            
            pthread_mutex_lock(&fielder_done_mutex);
            while (ball_in_air) {
              pthread_cond_wait(&fielder_done,&fielder_done_mutex);
            } 
            pthread_mutex_unlock(&fielder_done_mutex);
          } else {
            int runs = rand()%5;
            score+=runs;
            if (runs%2!=0) {
              break;
            }  
          }
        } 
      } else {
        // no ball
        score++;
        int hit = rand()%2; 
        if (hit) {
          int notOut = rand()%2; // out will be only through run out 
          if (notOut==0) {
            int runs = rand()%4;
            score+=runs;
            // make wicket type as run out and just do what was done for fielder 
          } else {
            int notBoundary = rand()%6;
            if (notBoundary==0) {
              BoundaryType what_boundary = rand()%2;
              if (what_boundary==FOUR) {
                score+=4;
              } else {
                score+=6;
              }
            } else {
              int runs = rand()%4;
              score+=runs;
              if (runs%2!=0) {
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
      }
      curr_ball = (curr_ball + 1) % BALL_BUF_SIZE;
      num_ball--;
      number_balls++;
      pthread_mutex_unlock(&pitch);
      pthread_cond_signal(&c_bo);
    } 
    if (notOut==0) {
      break;
    }
    sem_post(&active_end); // for odd runs - strike change 
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

void *fielding(void *param) {
  while(1) {
    pthread_mutex_lock(&fielder_mutex); 
    // prevent all fielders accesing ball in air 
    
    while(!ball_in_air) {
      pthread_cond_wait(&BALL_HIT,&fielder_mutex);
    }
  
    if (wicket_type==CAUGHT) {
      // stats here
    } else if (wicket_type==RUNOUT) {
      // stats here
    }
    //do work, then signal

    pthread_mutex_lock(&fielder_done_mutex);
    ball_in_air=0;
    pthread_mutex_unlock(&fielder_done_mutex);
    pthread_cond_signal(&fielder_done);

    pthread_mutex_unlock(&fielder_mutex);
  }
  return NULL;
}

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
  pthread_mutex_init(&fielder_mutex, NULL);
  pthread_cond_init(&fielder_done,NULL);
  pthread_mutex_init(&fielder_done_mutex, NULL);
  score = 0;

  new_batsman = 0;
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

