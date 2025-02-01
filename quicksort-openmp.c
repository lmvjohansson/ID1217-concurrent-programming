/* quicksort using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o quicksort-openmp quicksort-openmp.c 
     ./quicksort-openmp size numWorkers

*/

#include <omp.h>
#include <stdlib.h> /* To allow use of rand function */
#include <time.h> /* Only to allow random seed on the rand function */

double start_time, end_time;

#include <stdio.h>
#define MAXSIZE 5000000  /* maximum array size */
#define MAXWORKERS 8   /* maximum number of workers */

int numWorkers;
int size; 

/* Helper function to swap the contents of two pointers */
void swap(int *a, int *b) {
  int t = *a;
  *a = *b;
  *b = t;
}

/* Function that implements quicksort algorithm using omp for parallelism. It accepts an array and the indicies that mark the boudary of the part of the array to work on */
void quicksort(int array[], int low, int high) {
    if (low < high) { /* Terminaton condition, when low = high there is only one element left and the recursion should end */
        int pivot = array[high]; /* Pivot is arbitrarily chosen to be the last element in the subarray */
        int i = (low - 1); /* Variable that keeps track of the boundary between numbers lower and higher than pivot */
        for (int j = low; j < high; j++) {
            if (array[j] <= pivot) { /* If value is smaller than pivot place it in the front part of the subarray*/
                i++;
                swap(&array[i], &array[j]);
            }
        }
        swap(&array[i + 1], &array[high]); /* Once all elements of subarray has been sorted into smaller and bigger than pivot place pivot in correct position */
        pivot = i + 1;

        if ((high - low) > 50000){ /* Thershold to stop spawning too small parallel tasks that cause worse performance */      
            #pragma omp task /* Parallel task for each recusive call */
            quicksort(array, low, pivot - 1);

            #pragma omp task
            quicksort(array, pivot + 1, high);

            #pragma omp taskwait /* To ensure all tasks are allowed to complete */
        } else {
            quicksort(array, low, pivot - 1);
            quicksort(array, pivot + 1, high);
        }
    }
}

int main(int argc, char *argv[]) {
    int i;
    srand(time(NULL)); /* Added to get random seed so the matrix is not identical each time */

    /* read command line args if any */
    size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
    numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
    if (size > MAXSIZE) size = MAXSIZE;
    if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

    omp_set_num_threads(numWorkers);

    int *array = malloc(size * sizeof(int)); /* Create an populate array */
    for (i = 0; i < size; i++) {
        array[i] = rand()%1000000;
    }
  
    start_time = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single /* One thread starts the recursion */
        {
            quicksort(array, 0, size-1);
        }
    }

    end_time = omp_get_wtime(); 

    printf("The execution time is %g sec\n", end_time - start_time);

    #ifdef DEBUG
    int printout = size > 20 ? 20 : size;
    printf("[ %d", array[0]);
    for (i = 1; i < size; i++) {
        printf(", %d", array[i]);
    }
    printf(" ]\n");
    #endif

}