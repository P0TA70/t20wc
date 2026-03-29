#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ganttChart(int* output, int size,Team* ba) {
    char command[2048] = "python3 visualiser.py";
    
    // Append each array element to the command
    for (int i = 0; i < size; i++) {
        char temp[20];
        sprintf(temp, " %s",ba->players[output[i]].name);
        strcat(command, temp);
    }
    
    // Execute the Python script
    printf("Executing Gantt Chart visualization...\n");
    system(command);
}