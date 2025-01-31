/* quicksort function for array of ints

   features: spawns pthreads recursively

   usage under Windows:
     gcc -o quicksort quicksort.c -lpthread -DDEBUG
     quicksort size

   usage under Linux:
     gcc quicksort.c -lpthread
     a.out size

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

#define MAXSIZE 5000000;

void quicksort(int array[], int low, int high);
void *quicksortWorker(void* args);

double start_time, end_time; /* start and end times */
int arraySize;
pthread_attr_t attr;

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

/* Helper function to swap the contents of two pointers */
void swap(int *a, int *b) {
  int t = *a;
  *a = *b;
  *b = t;
}

/* Struct to contain the arguments required to call quicksort function since a thread can only be created with one pointer as argument */
struct Arguments {
    int *array;
    int low;
    int high;
};

/* Since the pthread is passed a struct a function is required to unpack the struct and call the quicksort function */
void *quicksortWorker(void* args) {
    struct Arguments* arguments = (struct Arguments*)args;
    quicksort(arguments->array, arguments->low, arguments->high);
    free(arguments);
    return NULL;
}

/* Function that implements quicksort algorithm using pthreads for parallelism. It accepts an array and the indicies that mark the boudary of the part of the array to work on */
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

        if ((high - low) > (arraySize / 16) && (high - low) > 50000) { /* Allowing the function to spawn threads for small subarrays causes the overhead of creating the thread to take longer than to let the program run sequentially */  
            pthread_t leftThread, rightThread;

            struct Arguments* leftArgs = malloc(sizeof(struct Arguments));
            leftArgs->array = array;
            leftArgs->low = low;
            leftArgs->high = pivot - 1;

            struct Arguments* rightArgs = malloc(sizeof(struct Arguments));
            rightArgs->array = array;
            rightArgs->low = pivot + 1;
            rightArgs->high = high;

            pthread_create(&leftThread, &attr, quicksortWorker, leftArgs);
            pthread_create(&rightThread, &attr, quicksortWorker, rightArgs);

            pthread_join(leftThread, NULL);
            pthread_join(rightThread, NULL);
        }
        else {
            quicksort(array, low, pivot - 1);
            quicksort(array, pivot + 1, high);
        }
    }
}

/* Function that implements quicksort algorithm normally. It accepts an array and the indicies that mark the boudary of the part of the array to work on */
void quicksortSequential(int array[], int low, int high) {
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

        quicksortSequential(array, low, pivot - 1);
        quicksortSequential(array, pivot + 1, high);
    }
}


int main(int argc, char *argv[]) {
    srand(time(NULL)); /* Added to get random seed so the array is not identical each time */
    int i;

    /* set global thread attributes */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    arraySize = (argc > 1)? atoi(argv[1]) : MAXSIZE;

    /* Two identical arrays are created */
    int *array = malloc(arraySize * sizeof(int));
    int *copy = malloc(arraySize * sizeof(int));
    for (i = 0; i < arraySize; i++) {
        array[i] = rand()%1000000;
        copy[i] = array[i];
    }

    /* Sequential quicksort is tested on the first array */
    start_time = read_timer();
    quicksortSequential(array, 0, arraySize - 1);
    end_time = read_timer();
    printf("The execution time for the regular quicksort is %g sec\n", end_time - start_time);

    /* Pthread quicksort is tested on the second array */
    start_time = read_timer();
    quicksort(copy, 0, arraySize - 1);
    end_time = read_timer();
    printf("The execution time for the pthread quicksort is %g sec\n", end_time - start_time);

    /* print the pthread quicksort array */
    #ifdef DEBUG
    int printout = arraySize > 20 ? 20 : arraySize;
    printf("[ %d", copy[0]);
    for (i = 1; i < arraySize; i++) {
        printf(", %d", copy[i]);
    }
    printf(" ]\n");
    #endif

    free(array);
    free(copy);
}



