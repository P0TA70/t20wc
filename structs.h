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

typedef struct BowlingScore
{
  int balls_delivered;
  int runs_given;
  int wickets_taken; 
} BowlingScore;
typedef struct BattingScore 
{
  int balls_received;
  int runs_taken;
  int sixes, fours;
} BattingScore;

typedef struct Probabilities {
  int boundary, out;
  int wide, legal;
  //^^ in order of 10^-4
  int death_over_specialist; //bool
} PDF;
typedef struct Player
{
  char* name;
  pthread_t tid;
  int team;
  BowlingScore bo_score;
  BattingScore ba_score;
  PDF pdf;
} Player;

typedef struct Team
{
  Player players[11];
  int team_score;
} Team;

#endif
