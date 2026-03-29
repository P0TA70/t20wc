main: main.c umpire.h umpire.c batting.c bowling.c fielding.c fifo_sem.h fifo_sem.c structs.h globals.h visualiser.c
	cc main.c umpire.c fifo_sem.c batting.c bowling.c fielding.c visualiser.c
