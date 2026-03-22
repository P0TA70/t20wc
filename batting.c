#include "globals.h"
#include <stdlib.h>


void *batting(void *param) {
  fifo_sem_wait(&crease);

  int got_out = 0; // adding ts cuz when previously a batsman got out, crease was being posted twice
                   // extra post was allowing 3rd batsman to come!

  while (1) {
    pthread_mutex_lock(&nb_mutex);
    if (!new_batsman) {
      pthread_mutex_unlock(&nb_mutex);
      sem_wait(&passive_end);
      sem_wait(&active_end);
      sem_post(&passive_end);
    } else {
      new_batsman=0;
      pthread_mutex_unlock(&nb_mutex);
    }

    // critical section
    int notOut = 1; // wide and no ball case was leaving ts uninitialized, so i added it!
    int over_end = 0; // to check if the current over has ended!
    while (1) {
      pthread_mutex_lock(&pitch);
      if (num_ball < 0) {
        printf("crash");
        fflush(stdout);
        exit(1);
      } /* underflow */
      while (num_ball == 0) /* block if buffer empty */
        pthread_cond_wait(&c_ba, &pitch);
      /* if executing here, buffer not empty so remove element */
      printf("%d #%lx %d \n", number_balls, pthread_self(), balls[curr_ball]);
      fflush(stdout);

      // deciding how batsman hit the ball
      if (balls[curr_ball] == LEGAL_BALL) {
        balls_in_over++;
        // fair ball
        notOut = rand() % 15;
        if (notOut == 0) {
          WicketType typeOfWicket = rand() % 7;
          // TODO: add more cases here
          if (typeOfWicket==CAUGHT) {
            pthread_mutex_lock(&fielder_mutex);
            ball_in_air=1;
            wicket_type=typeOfWicket; 
            pthread_mutex_unlock(&fielder_mutex);
            pthread_cond_signal(&BALL_HIT);
            
            pthread_mutex_lock(&fielder_done_mutex);
            while (ball_in_air) {
              pthread_cond_wait(&fielder_done,&fielder_done_mutex);
            } 
            pthread_mutex_unlock(&fielder_done_mutex);
          } else if (typeOfWicket==RUNOUT) {
            int runs = rand()%4;
            score+=runs;

            pthread_mutex_lock(&fielder_mutex);
            ball_in_air=1;
            wicket_type=typeOfWicket; 
            pthread_mutex_unlock(&fielder_mutex);
            pthread_cond_signal(&BALL_HIT);
            
            pthread_mutex_lock(&fielder_done_mutex);
            while (ball_in_air) {
              pthread_cond_wait(&fielder_done,&fielder_done_mutex);
            } 
            pthread_mutex_unlock(&fielder_done_mutex);
          } 
          
          bowler_stats[over_count].balls_delivered++;
          bowler_stats[over_count].wickets_taken++;

          curr_ball = (curr_ball + 1) % BALL_BUF_SIZE; // !!!
          num_ball--;                                    
          number_balls++;                                
          pthread_mutex_unlock(&pitch);  // we were not releasing this earlier!  
          pthread_cond_signal(&c_bo);                    

          pthread_mutex_lock(&nb_mutex);
          new_batsman = 1;
          pthread_mutex_unlock(&nb_mutex);

          got_out = 1; // this is to prevent double post of that crease semaphore!
          fifo_sem_post(&crease);
          if (!new_batsman) {
            sem_post(&active_end);
          } else {
            break;
          }
          pthread_exit(NULL);
        } else {
          int boundary = rand() % 9;
          if (boundary < 2) {
            BoundaryType bt = rand() % 2;
            if (bt == FOUR) {
              score += 4;
            } else {
              score += 6;
            }
          } else {
            int score_increase = rand() % 4;
            score+=score_increase;
            if (score_increase%2!=0) {
              curr_ball = (curr_ball + 1) % BALL_BUF_SIZE; // forgor to consume the ball here too!
              num_ball--;                                    
              number_balls++;                                
              pthread_mutex_unlock(&pitch);                  // we were not releasing this earlier!
              pthread_cond_signal(&c_bo);                    
              break; 
            }
          }
        }
      } else if (balls[curr_ball] == WIDE) {
        // wide ball
        score+=1;
        int not_overthrow = rand()%10;
        if (!not_overthrow) {
          // overthrow case here 
          int notRunOut = rand()%4;
          if (notRunOut==0) {
            int runs = rand()%4;
            score+=runs;

            pthread_mutex_lock(&fielder_mutex);
            ball_in_air=1;
            wicket_type=RUNOUT; 
            pthread_mutex_unlock(&fielder_mutex);
            pthread_cond_signal(&BALL_HIT);
            
            pthread_mutex_lock(&fielder_done_mutex);
            while (ball_in_air) {
              pthread_cond_wait(&fielder_done,&fielder_done_mutex);
            } 
            pthread_mutex_unlock(&fielder_done_mutex);
          } else {
            int runs = rand()%5;
            score+=runs;
            if (runs%2!=0) {
              curr_ball = (curr_ball + 1) % BALL_BUF_SIZE; // same thing!
              num_ball--;                                     
              number_balls++;                                
              pthread_mutex_unlock(&pitch);                  
              pthread_cond_signal(&c_bo);                    
              break;
            }  
          }
        } 
      } else {
        // no ball
        score++;
        int hit = rand()%2; 
        if (hit) {
          int notOut = rand()%2; // out will be only through run out 
          if (notOut==0) {
            int runs = rand()%4;
            score+=runs;
            // make wicket type as run out and just do what was done for fielder 
          } else {
            int notBoundary = rand()%6;
            if (notBoundary==0) {
              BoundaryType what_boundary = rand()%2;
              if (what_boundary==FOUR) {
                score+=4;
              } else {
                score+=6;
              }
            } else {
              int runs = rand()%4;
              score+=runs;
              if (runs%2!=0) {
                curr_ball = (curr_ball + 1) % BALL_BUF_SIZE; // same thing, consume the ball in no ball case, otherwise the 
                                                             // same ball was being read in this case!
                num_ball--;                                    
                number_balls++;                                
                pthread_mutex_unlock(&pitch);                  // you know at this point
                pthread_cond_signal(&c_bo);                    // im tired!
                break;
              }
            }
          }
        }
        // freeHit();  // TODO 
      }

      if (balls_in_over == 6) {
        balls_in_over = 0;
        over_count++;
        over_end = 1; // flag to signify over has ended!
      }
      curr_ball = (curr_ball + 1) % BALL_BUF_SIZE;
      num_ball--;
      number_balls++;
      pthread_mutex_unlock(&pitch);
      pthread_cond_signal(&c_bo);
      if(over_end) {
        sem_post(&active_end);
        break;  // swap sides of the pitch!
      }
    } 
    if (notOut==0) {
      break;
    }
    if(!over_end) sem_post(&active_end); // for odd runs side change!
  }
  if (!got_out) {                  // post crease only if not posted above!
    fifo_sem_post(&crease);       
  }
  return NULL;
}

