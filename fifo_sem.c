#include "fifo_sem.h"
#include <unistd.h>
#include <pthread.h>

void fifo_sem_init(fifo_sem *sem, int val) {
  sem->value = val;
  sem->end_of_queue = 0;
  sem->next_consumer = 0;
  pthread_mutex_init(&sem->lock, NULL);
  pthread_cond_init(&sem->turn_in_queue, NULL);
}

void fifo_sem_wait(fifo_sem *sem) {
  pthread_mutex_lock(&sem->lock);

  int index = sem->end_of_queue; // give a new index to this thread
  sem->end_of_queue++; // increase it so another thread gets the next index

  while (index != sem->next_consumer || sem->value == 0) {
    pthread_cond_wait(&sem->turn_in_queue, &sem->lock);
  }

  sem->next_consumer++;
  sem->value--;

  pthread_cond_broadcast(&sem->turn_in_queue);
  pthread_mutex_unlock(&sem->lock);
}

void fifo_sem_post(fifo_sem *sem) {
  pthread_mutex_lock(&sem->lock);
  sem->value++;
  pthread_cond_broadcast(&sem->turn_in_queue);
  pthread_mutex_unlock(&sem->lock);
}
