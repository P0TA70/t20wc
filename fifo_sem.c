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

//example usage
/*
#include <stdio.h>
#include <stdlib.h>
fifo_sem sem;
void* function(void* param) {
  int sleep_time = atoi(param);
  printf("%d\n", sleep_time);
  fifo_sem_wait(&sem);
  printf("hi from thread %d\n", sleep_time);
  fifo_sem_post(&sem);
  return NULL;
}
int main() {
  fifo_sem_init(&sem, 1);
  pthread_t a,b,c,d;
  pthread_create(&a, NULL, function, "1");
  pthread_create(&b, NULL, function, "2");
  pthread_create(&c, NULL, function, "3");
  pthread_create(&d, NULL, function, "4");
  pthread_join(a, NULL);
  pthread_join(b, NULL);
  pthread_join(c, NULL);
  pthread_join(d, NULL);
}
*/

