#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include "structs.h"
#include "fifo_sem.h"

#define BALL_BUF_SIZE 42 //6*7

//zhe ground
extern fifo_sem crease;
extern sem_t active_end, passive_end;

//match stats
//extern pthread_mutex_t score_mut;
extern int score; //threads: batter
extern int wickets; //bowler
extern int balls_in_over; //bowler
extern int over_count; // vumpire
//extern int innings_complete; //umpire
extern WicketType wicket_type;
// mutex for wicket type is fielder mutex

//current situation
//ignoring run mutex for now
extern BallOutcome ball_outcome; //bowler
extern StrokeOutcome stroke_outcome; //batsman

//player signals 
extern pthread_mutex_t fielder_mutex;
// extern pthread_cond_t BALL_THROW; 
extern int ball_in_air;
extern pthread_cond_t BALL_HIT;
// batsman will wait on a condition variable from fielder
extern pthread_cond_t fielder_done;
extern pthread_mutex_t fielder_done_mutex;
// we need fielder done mutex and fielder mutex seperately cause TRUST!

extern int new_ball, curr_ball, num_ball;
extern pthread_mutex_t pitch;
extern pthread_cond_t c_ba, c_bo;
extern BallOutcome balls[BALL_BUF_SIZE];

// Order of Bowlers 
extern pthread_t bowlers[20];

// for when a new batsman enters (double while)
extern int new_batsman;
extern pthread_mutex_t nb_mutex;

// D BUGGER added flaag so fielders may die!
extern int match_over;

extern int number_balls;
extern BowlingScore bowler_stats[20];

#endif
