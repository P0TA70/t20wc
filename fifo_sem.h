#ifndef FIFO_SEM_H
#define FIFO_SEM_H

#include <pthread.h>

typedef struct FifoSem {
  int value;
  int end_of_queue; // when another thread comes, it gets assigned this index in
                    // queue, which increments
  int next_consumer; // this is the queue index corresponding to the thread
                     // currently running
  pthread_mutex_t lock;
  pthread_cond_t turn_in_queue;
} fifo_sem;

void fifo_sem_init(fifo_sem *sem, int val);
void fifo_sem_wait(fifo_sem *sem);
void fifo_sem_post(fifo_sem *sem);

#endif
