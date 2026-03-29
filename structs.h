#ifndef STRUCTS_H
#define STRUCTS_H

#include <pthread.h>

typedef enum BallOutcome {
  LEGAL_BALL, // 0 
  WIDE, // 1
  NO_BALL, // 2
} BallOutcome;

typedef enum WicketType {
  BOWLED,
  RUNOUT,
  CAUGHT,
  LBW,
  STUMPED, //might remove
  HIT_WICKET, //might remove
  OBSTRUCTING_FIELD, //might remove
  HIT_TWICE, // might remove
} WicketType;
typedef enum BoundaryType {
  FOUR,
  SIX,
} BoundaryType;

typedef struct StrokeOutcome {
  int wicket_bool;
  WicketType wicket_type;
  int boundary_bool;
  BoundaryType boundary_type;
  int runs;
} StrokeOutcome;

typedef struct Probabilities {
  int boundary, out;
  int wide, legal;
  //^^ in order of 10^-4
  int death_over_specialist; //bool
} PDF;

typedef struct BatterResults {
  char* batter_names[11];
  int wait_balls[11];
  int score;
} Results;

typedef struct Player
{
  char* name;
  pthread_t tid;
  int team;
  PDF pdf;
} Player;

typedef struct Team
{
  Player players[11];
  int team_score;
} Team;

#endif
