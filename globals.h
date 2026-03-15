#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include "structs.h"

//zhe ground
extern sem_t crease;
extern sem_t active_end, passive_end;
extern sem_t is_bowling;

//match stats
extern pthread_mutex_t score_mut;
extern int score; //threads: bowler, batter
extern int wickets; //bowler
extern int balls_in_over; //bowler
extern int over_count; // vumpire
extern int innings_complete; //umpire

//current situation
//ignoring run mutex for now
extern BallOutcome ball_outcome; //bowler
extern StrokeOutcome stroke_outcome; //batsman

//player signals
extern pthread_mutex_t pitch_mut;
extern pthread_cond_t BALL_THROW;
extern pthread_cond_t BALL_HIT;
extern pthread_cond_t BALL_COMPLETE;

extern FILE *ballsInOver;

#endif
