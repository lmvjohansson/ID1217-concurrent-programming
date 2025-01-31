/* matrix summation, min asnd max using pthreads

   features: uses a bag of tasks and pthread_join before calculating the global min, max and sum in main thread.
   
   usage under Windows:
     gcc -o matrixSum matrixSum.c -lpthread
     matrixSum size numWorkers

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

pthread_mutex_t nextRowLock; /* Mutex to protect accessing the nextRow variable */
int nextRow = 0; /* Global variable to keep track of next row to work on in the matrix, this is the bag of tasks */

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

/* struct to encapsulate the result, returns total sum, min value and its position, max value and its position */
struct Result {
  int total;
  int minimum;
  int minRow;
  int minColumn;
  int maximum;
  int maxRow;
  int maxColumn;
};

double start_time, end_time; /* start and end times */
int size, numWorkers;  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l,k; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];
  struct Result globalResult;
  srand(time(NULL)); /* Added to get random seed so the matrix is not identical each time */

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&nextRowLock, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%99;
	  }
  }

  /* Initialize the Result struct */
  globalResult.total = 0;
  globalResult.minimum = matrix[0][0];
  globalResult.minRow = 0;
  globalResult.minColumn = 0;
  globalResult.maximum = matrix[0][0];
  globalResult.maxRow = 0;
  globalResult.maxColumn = 0;

  /* print the matrix */
 #ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
 #endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++) {
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  }
  for (k = 0; k < numWorkers; k++){
    struct Result *threadResult;
    pthread_join(workerid[k], (void **) &threadResult);
    globalResult.total += threadResult->total;
      if (threadResult->minimum < globalResult.minimum){
        globalResult.minimum = threadResult->minimum;
        globalResult.minRow = threadResult->minRow;
        globalResult.minColumn = threadResult->minColumn;
      }
      if (threadResult->maximum > globalResult.maximum){
        globalResult.maximum = threadResult->maximum;
        globalResult.maxRow = threadResult->maxRow;
        globalResult.maxColumn = threadResult->maxColumn;
      }
    free(threadResult);
  }
      /* get end time */
    end_time = read_timer();
    /* print results */
    printf("The total is %d\n", globalResult.total);
    printf("The minimum value is %d, located at %d,%d\n", globalResult.minimum, globalResult.minRow, globalResult.minColumn);
    printf("The maximum value is %d, located at %d,%d\n", globalResult.maximum, globalResult.maxRow, globalResult.maxColumn);
    printf("The execution time is %g sec\n", end_time - start_time);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int i, j;
  struct Result *result = malloc(sizeof(struct Result)); /* pthread_join expects a pointer to the result so result is initialized as a pointer */

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* initialize struct with values, since result is a pointer we have to use -> to access the members of the struct */
  result->total = 0;
  result->minimum = matrix[0][0];
  result->minRow = 0;
  result->minColumn = 0;
  result->maximum = matrix[0][0];
  result->maxRow = 0;
  result->maxColumn = 0;

  while (true){  
    pthread_mutex_lock(&nextRowLock);
    if (nextRow >= size) { /* If the newRow counter has reached the size of the matrix it is time for the threads to break while loop and return results */
      pthread_mutex_unlock(&nextRowLock);
      break;
    }
    i = nextRow; /* Lock is acquired and there are still rows to be worked on, thread takes the nextRow, increases the counter and releases the lock for next thread */
    nextRow++;
    pthread_mutex_unlock(&nextRowLock);

    /* sum values, calculates min and max */
    for (j = 0; j < size; j++) {
      result->total += matrix[i][j]; /* Updates partial sum */
      if (matrix[i][j] < result->minimum){ /* Checks if current entry is smaller than min, if so it is recorded */
        result->minimum = matrix[i][j];
        result->minRow = i;
        result->minColumn = j;
      }
      if (matrix[i][j] > result->maximum){ /* Check if current entry is larger than max, if so it is recorded */
        result->maximum = matrix[i][j];
        result->maxRow = i;
        result->maxColumn = j;
      }
    }
  }

  return result; /* Changes made to Worker function - removed call to barrier function, the total summation and the stopping of the clock. */
}

