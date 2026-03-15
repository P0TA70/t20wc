#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include "structs.h"

#define BALL_BUF_SIZE 42 //6*7

//zhe ground
extern sem_t crease;
extern sem_t active_end, passive_end;

//match stats
//extern pthread_mutex_t score_mut;
extern int score; //threads: batter
extern int wickets; //bowler
extern int balls_in_over; //bowler
extern int over_count; // vumpire
//extern int innings_complete; //umpire

//current situation
//ignoring run mutex for now
extern BallOutcome ball_outcome; //bowler
extern StrokeOutcome stroke_outcome; //batsman

//player signals 
/*extern pthread_mutex_t pitch_mut;
extern pthread_cond_t BALL_THROW; */
extern pthread_cond_t BALL_HIT;
// extern pthread_cond_t BALL_COMPLETE;

extern int new_ball, curr_ball, num_ball;
extern pthread_mutex_t pitch;
extern pthread_cond_t c_ba, c_bo;
extern BallOutcome balls[BALL_BUF_SIZE];

// Order of Bowlers 
extern pthread_t bowlers[20];

#endif
