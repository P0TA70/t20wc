#include "structs.h"
#include "umpire.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  srand(time(NULL));

  printf("here i am");
  // some input data is converted to 2 teams
  Team *a = construct_team(0);
  Team *b = construct_team(1);

  int coin = rand() % 2;
  if (coin) {
    umpire(a, b);
  } else {
    umpire(b, a);
  }

  return 0;
}

