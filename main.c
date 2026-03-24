#include "structs.h"
#include "umpire.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  srand(time(NULL));

  // some input data is converted to 2 teams
  Team *a = construct_team(0);
  Team *b = construct_team(1);

  for (int i=0;i<11;i++) {
    PDF* pdf = &a->players[i].pdf;
    printf("My name is %s, %d %d %d %d %d\n", a->players[i].name, pdf->boundary, pdf->out, pdf->legal, pdf->wide, pdf->death_over_specialist);
  }

  int coin = rand() % 2;
  if (coin) {
    umpire(a, b);
  } else {
    umpire(b, a);
  }

  return 0;
}

Team *construct_team(int data /*dummy, team bool for now*/) {
  // pretend some  probability stuff assigns proper stats
  Team *team = malloc(sizeof(Team));
  team->team_score = 0;

  FILE *fp = (data == 0) ? fopen("team1.txt", "r") : fopen("team2.txt", "r");
  if (fp == NULL) {
    perror("cannot open file");
    exit(1);
  }

  for (int i = 0; i < 11; i++) {
    Player* current = &team->players[i];
    
    current->team = data;
    current->bo_score.balls_delivered = 0;
    current->bo_score.runs_given = 0;
    current->bo_score.wickets_taken = 0;
    current->ba_score.runs_taken = 0;
    current->ba_score.balls_received = 0;
    current->ba_score.fours = 0;
    current->ba_score.sixes = 0;

    PDF *pdf = &current->pdf;
    current->name = malloc(48);
    char buffer[1001];
    fgets(buffer, 1000, fp);
    sscanf(buffer, "%48s %d %d %d %d %d", current->name, &pdf->boundary, &pdf->out, &pdf->legal, &pdf->wide, &pdf->death_over_specialist);
  }
  return team;
}
