#include "structs.h"
#include "umpire.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  srand(time(NULL));

  Team *a = construct_team(0);
  Team *b = construct_team(1);

  printf("TEAM 1 PLAYERS\n");
  for (int i = 0; i < 11; i++) {
    PDF *pdf = &a->players[i].pdf;
    printf("My name is %s, %d %d %d %d %d\n", a->players[i].name, pdf->boundary,
           pdf->out, pdf->legal, pdf->wide, pdf->death_over_specialist);
  }

  printf("\n");

  printf("TEAM 2 PLAYERS\n");
  for (int i = 0; i < 11; i++) {
    PDF *pdf = &b->players[i].pdf;
    printf("My name is %s, %d %d %d %d %d\n", b->players[i].name, pdf->boundary,
           pdf->out, pdf->legal, pdf->wide, pdf->death_over_specialist);
  }

  int coin = rand() % 2;
  Results first_innings_score, second_innings_score;
  Results sjf_results;
  printf("FCFS batter scheduling\n");
  printf("\nFirst innings beginning with team %d\n", coin + 1);
  if (!coin) {
    first_innings_score = umpire(a, b, 0, INT_MAX);
    printf("\nSecond innings beginning with team 2\n");
    second_innings_score = umpire(b, a, 0, first_innings_score.score);

    freopen("sjf.txt", "w", stdout);
    sjf_results = umpire(a, b, 1, INT_MAX);
    freopen("/dev/tty", "w", stdout);

  } else {
    first_innings_score = umpire(b, a, 0, INT_MAX);
    printf("\nSecond innings beginning with team 1\n");
    second_innings_score = umpire(a, b, 0, first_innings_score.score);

    freopen("sjf.txt", "w", stdout);
    sjf_results = umpire(b, a, 1, INT_MAX);
    freopen("/dev/tty", "w", stdout);
  }

  printf("\n\nTeam %d played first and scores %d runs!\n", coin + 1,
         first_innings_score.score);
  printf("Team %d played second and scores %d runs!\n", 2 - coin,
         second_innings_score.score);

  printf("\nCheck sjf.txt for log describing if SJF was used for batting "
         "scheduling, for the team that played first.\n");
  printf("MIDDLE ORDER WAIT ANALYSIS\n");
  printf("%-12s  %10s  %10s\n", "Player", "FCFS wait time", "SJF wait time");

  int average1 = 0;
  int average2 = 0;
  for (int i = 0; i < 4; i++) {
    average1 += first_innings_score.wait_balls[3 + i];
    average2 += sjf_results.wait_balls[i + 3];
    printf("%-12d  %10d  %10d\n", i + 4, first_innings_score.wait_balls[3 + i],
           sjf_results.wait_balls[i + 3]);
  }
  return 0;
}

Team *construct_team(int data ) {
  Team *team = malloc(sizeof(Team));
  team->team_score = 0;

  FILE *fp = (data == 0) ? fopen("team1.txt", "r") : fopen("team2.txt", "r");
  if (fp == NULL) {
    perror("cannot open file");
    exit(1);
  }

  for (int i = 0; i < 11; i++) {
    Player *current = &team->players[i];

    current->team = data;
    current->id=i;

    PDF *pdf = &current->pdf;
    current->name = malloc(48);
    char buffer[1001];
    fgets(buffer, 1000, fp);
    sscanf(buffer, "%48s %d %d %d %d %d", current->name, &pdf->boundary,
           &pdf->out, &pdf->legal, &pdf->wide, &pdf->death_over_specialist);
  }
  return team;
}
