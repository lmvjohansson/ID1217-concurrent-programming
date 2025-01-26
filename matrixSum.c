/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

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
#define MAXSIZE 100000  /* maximum matrix size */
#define MAXWORKERS 4   /* maximum number of workers */

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */ 
int numArrived = 0;       /* number who have arrived */

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

/* struct to encapsulate return of a worker, returns total partial sum, min value and its position, max value and its position */
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
int size, stripSize;  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l,k; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];
  struct Result globalResult;
  //srand(time(NULL));

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

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
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
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
  int total, i, j, first, last;
  struct Result *result = malloc(sizeof(struct Result)); /* pthread_join expects a pointer to the result so result is initialized as a pointer */

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* initialize struct with values, since result is a pointer we have to use -> to access the members of the struct */
  result->total = 0;
  result->minimum = matrix[first][0];
  result->minRow = first;
  result->minColumn = 0;
  result->maximum = matrix[first][0];
  result->maxRow = first;
  result->maxColumn = 0;

  /* sum values in my strip */
  for (i = first; i <= last; i++) {
    for (j = 0; j < size; j++) {
      result->total += matrix[i][j];
      if (matrix[i][j] < result->minimum){
        result->minimum = matrix[i][j];
        result->minRow = i+1;
        result->minColumn = j+1;
      }
      if (matrix[i][j] > result->maximum){
        result->maximum = matrix[i][j];
        result->maxRow = i+1;
        result->maxColumn = j+1;
      }
    }
  }

  return result; /* Changes made to Worker function - removed call to barrier function, the total summation and the stopping of the clock. */
}

