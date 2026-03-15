#ifndef STRUCTS_H
#define STRUCTS_H

#include <pthread.h>

typedef enum BallOutcome {
  LEGAL_BALL,
  WIDE,
  NO_BALL,
} BallOutcome;

typedef enum WicketType {
  BOWLED,
  RUNOUT,
  CAUGHT,
  LBW,
  STUMPED, //might remove
  HIT_WICKET, //might remove
  OBSTRUCTING_FIELD, //might remove
} WicketType;
typedef enum BoundaryType {
  FOUR,
  SIX,
} BoundaryType;

typedef struct StrokeOutcome {
  WicketType wicket_type;
  int runs;
  BoundaryType boundary_type;
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

typedef struct Player
{
  pthread_t id;
  int team;
  BowlingScore bo_score;
  BattingScore ba_score;
  int death_over_specialist;
} Player;

typedef struct Team
{
  Player players[11];
  int team_score;
} Team;

#endif
