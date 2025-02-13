/* program to simulate unisex bathroom problem

   usage under Windows:
     gcc -o unisex-bathroom unisex-bathroom.c -lpthread -lposix4
     unisex-bathroom numberOfOneSex
*/

#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define MAXPERSONS 20;

sem_t accessMutex, /* Ensures only men or only women access bathroom */
      womenCountMutex, /* Protects count variable */
      menCountMutex, /* Protects count variable */
      queueMutex; /* Ensures fairness between sexes */

int womenCount = 0, menCount = 0; /* Keeps track of number of persons of each sex in the bathroom, good states: (womenCount >= 0 and menCount == 0) or (womenCount == 0 and menCount >= 0) */

void *Women(void *); /* The two thread functions */
void *Men(void *);


int main(int argc, char *argv[]) {
  int persons = (argc > 1)? atoi(argv[1]) : MAXPERSONS;

  sem_init(&accessMutex, 0, 1); /* All semaphores start in an unlocked state and pshared is set to shared between threads (not processes) */
  sem_init(&womenCountMutex, 0, 1); 
  sem_init(&menCountMutex, 0, 1); 
  sem_init(&queueMutex, 0, 1); 
  
  srand(time(NULL)); /* To allow for random seed generation */

  pthread_t workerid[persons * 2];

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); /* Sets attributes of pthread to allow system to schedule execution */

  for (int i = 0; i < persons; i++) {
    pthread_create(&workerid[i], &attr, Women, NULL);
    pthread_create(&workerid[(i + persons)], &attr, Men, NULL);
  }

  for (int i = 0; i < (persons * 2); i++) {
    pthread_join(workerid[i], NULL);
  }

  sem_destroy(&accessMutex);
  sem_destroy(&womenCountMutex);
  sem_destroy(&menCountMutex);
  sem_destroy(&queueMutex);
}

void *Women(void *) {
  unsigned int seed = (unsigned int) pthread_self(); /* Thread specific seed to allow the sleeps to actually be random */
  while(1) {
    sem_wait(&queueMutex); /* Attempts to enter bathroom, if the other sex is inside or there is a queue person will wait */
    sem_wait(&womenCountMutex); /* Attempt to access the counter variables */

    if (womenCount == 0) { /* If this is the first woman she will take the accessMutex and prevent men from entering */
        sem_wait(&accessMutex);
    }
    womenCount++; /* Enter bathroom */
    printf("Woman %d enters bathroom, number of women in bathroom is %d\n", pthread_self(), womenCount);

    sem_post(&womenCountMutex); /* Release lock on count */
    sem_post(&queueMutex); /* Allow women to enter bathroom or men to queue up */

    srand(seed);
    sleep(rand() % 4 + 1); /* Simulate using bathroom */

    sem_wait(&womenCountMutex); /* Attempt to access count variable */
    womenCount--; /* Leave bathroom */
    printf("Woman %d leaves bathroom, number of women in bathroom is %d\n", pthread_self(), womenCount);

    if (womenCount == 0) { /* If the last woman leaves unlock access to bathroom for any sex */
      sem_post(&accessMutex);
    }

    sem_post(&womenCountMutex); /* Release lock on count */

    srand(seed);
    sleep(rand() % 4 + 1); /* Wait some time before using bathroom again */
  }
}

void *Men(void *) {
  unsigned int seed = (unsigned int) pthread_self();
  while(1) {
    sem_wait(&queueMutex);
    sem_wait(&menCountMutex);

    if (menCount == 0) {
        sem_wait(&accessMutex);
    }
    menCount++;
    printf("Man %d enters bathroom, number of men in bathroom is %d\n", pthread_self(), menCount);

    sem_post(&menCountMutex);
    sem_post(&queueMutex);

    srand(seed);
    sleep(rand() % 4 + 1);

    sem_wait(&menCountMutex);
    menCount--; 
    printf("Man %d leaves bathroom, number of men in bathroom is %d\n", pthread_self(), menCount);

    if (menCount == 0) {
      sem_post(&accessMutex);
    }

    sem_post(&menCountMutex);

    srand(seed);
    sleep(rand() % 4 + 1);
  }
}