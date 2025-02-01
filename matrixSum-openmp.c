/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c 
     ./matrixSum-openmp size numWorkers

*/

#include <omp.h>
#include <stdlib.h> /* To allow use of rand function */
#include <time.h> /* Only to allow random seed on the rand function */

double start_time, end_time;

#include <stdio.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 8   /* maximum number of workers */

int numWorkers;
int size; 
int matrix[MAXSIZE][MAXSIZE];
void *Worker(void *);

/* struct to encapsulate the result, returns min value and its position, max value and its position */
struct Result {
  int minimum;
  int minRow;
  int minColumn;
  int maximum;
  int maxRow;
  int maxColumn;
};

int main(int argc, char *argv[]) {
  int i, j, total=0;
  struct Result globalResult;
  srand(time(NULL)); /* Added to get random seed so the matrix is not identical each time */

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
    //  printf("[ ");
	  for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
      //	  printf(" %d", matrix[i][j]);
	  }
	  //	  printf(" ]\n");
  }

  /* Initialize the globalResult struct */
  globalResult.minimum = matrix[0][0];
  globalResult.minRow = 0;
  globalResult.minColumn = 0;
  globalResult.maximum = matrix[0][0];
  globalResult.maxRow = 0;
  globalResult.maxColumn = 0;
  
  start_time = omp_get_wtime();

  #pragma omp parallel
  {
    struct Result localResult; /* This will be a private variable in each thread */
    localResult.minimum = matrix[0][0];
    localResult.minRow = 0;
    localResult.minColumn = 0;
    localResult.maximum = matrix[0][0];
    localResult.maxRow = 0;
    localResult.maxColumn = 0;

    #pragma omp for reduction (+:total) private(j) /* Moved parallel command to top omp statement to avoid nesting of parallel execution */
    for (i = 0; i < size; i++){
      for (j = 0; j < size; j++){
        total += matrix[i][j];

        if (matrix[i][j] < localResult.minimum){ /* Checks if current entry is smaller than min, if so it is recorded */
          localResult.minimum = matrix[i][j];
          localResult.minRow = i;
          localResult.minColumn = j;
        }
        if (matrix[i][j] > localResult.maximum){ /* Check if current entry is larger than max, if so it is recorded */
          localResult.maximum = matrix[i][j];
          localResult.maxRow = i;
          localResult.maxColumn = j;
        }
      }
    }

    end_time = omp_get_wtime(); /* Instructions said to only take time for parallel part, critical section is not parallel */

    #pragma omp critical /* Critical sectio to ensure globalResult is updated correctly */
    {
      if (localResult.minimum < globalResult.minimum){
        globalResult.minimum = localResult.minimum;
        globalResult.minRow = localResult.minRow;
        globalResult.minColumn = localResult.minColumn;
      }
      if (localResult.maximum > globalResult.maximum){
        globalResult.maximum = localResult.maximum;
        globalResult.maxRow = localResult.maxRow;
        globalResult.maxColumn = localResult.maxColumn;
      }
    }
  }

  printf("the total is %d\n", total);
    printf("The minimum value is %d, located at %d,%d\n", globalResult.minimum, globalResult.minRow, globalResult.minColumn);
    printf("The maximum value is %d, located at %d,%d\n", globalResult.maximum, globalResult.maxRow, globalResult.maxColumn);
    printf("The execution time is %g sec\n", end_time - start_time);

}