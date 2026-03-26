#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include "structs.h"
#include "fifo_sem.h"

#define BALL_BUF_SIZE 42 //6*7

extern int bowlers[20]; // Order of Bowlers 
extern BowlingScore bowler_stats[20]; //same order as above


extern BallOutcome balls[BALL_BUF_SIZE]; //shared buffer and its variables to access
extern int new_ball, curr_ball, num_ball;
extern pthread_mutex_t pitch;
extern pthread_cond_t c_ba, c_bo;


extern fifo_sem crease; //for batsman
extern sem_t active_end, passive_end;

extern int balls_in_over; //batsman iterates over balls using this 
extern int over_count;  
extern int number_balls;

extern int score; 
extern int wickets; 

extern int new_batsman;
extern pthread_mutex_t nb_mutex;


extern WicketType wicket_type;

extern int ball_in_air;
extern pthread_cond_t BALL_HIT;
// batsman will wait on a condition variable from fielder
extern pthread_mutex_t fielder_mutex;
extern pthread_cond_t fielder_done;
extern pthread_mutex_t fielder_done_mutex;
// we need fielder done mutex and fielder mutex seperately cause TRUST!
// for when a new batsman enters (double while)
// D BUGGER added flaag so fielders may die!
extern int match_over;

// Deadlocker vars in runout
extern pthread_mutex_t runout_mutex;
extern int runout_occuring;


#endif
