/***************************************************************************
 *
 * Parallel version of Quick sort
 *
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define THREADS 8192            // = Number of threads to be created
#define KILO (1024)          // = 1024
#define MEGA (1024*1024)     // = 1 048 576
#define MAX_ITEMS (64*MEGA)  // = 67 108 864

// pthread_mutex_t lock;
// static int counter = 1;

#define swap(v, a, b) {unsigned tmp; tmp=v[a]; v[a]=v[b]; v[b]=tmp;}

static int *v;

struct threadArgs
{
    int low;
    int high;
    int numThreads;
};

static void
print_array(void)
{
    int i;
    for (i = 0; i < MAX_ITEMS; i++)
        printf("%d ", v[i]);
    printf("\n");
}

static void
init_array(void)
{
    int i;
    v = (int *) malloc(MAX_ITEMS*sizeof(int));
    for (i = 0; i < MAX_ITEMS; i++)
        v[i] = rand() % MAX_ITEMS + 1;
}

static unsigned
partition(int *v, unsigned low, unsigned high, unsigned pivot_index)
{
    /* move pivot to the bottom of the vector */
    if (pivot_index != low)
        swap(v, low, pivot_index);

    pivot_index = low;
    low++;

    /* invariant:
     * v[i] for i less than low are less than or equal to pivot
     * v[i] for i greater than high are greater than pivot
     */

    /* move elements into place */
    while (low <= high) {
        if (v[low] <= v[pivot_index])
            low++;
        else if (v[high] > v[pivot_index])
            high--;
        else
            swap(v, low, high);
    }

    /* put pivot back between two groups */
    if (high != pivot_index)
        swap(v, pivot_index, high);
    return high;
}

void*
quick_sort(void* params)
{
    struct threadArgs* args = (struct threadArgs*) params;

    int pivot_index;
    int low = args->low;
    int high = args->high;
    int numThreads = args->numThreads;

    if (low >= high) // No need to sort a vector of zero or one element
        return NULL;

    pivot_index = (low + high) / 2; // Select pivot point
    pivot_index = partition(v, low, high, pivot_index); // Partition the vector

    struct threadArgs args_l, args_r;


    if(numThreads > 2) // If allowed, it should create more threads
    {
        // pthread_mutex_lock(&lock);
        // counter+=2;
        // pthread_mutex_unlock(&lock);
        args_l = (struct threadArgs){low, pivot_index - 1, args->numThreads / 2};
        args_r = (struct threadArgs){pivot_index + 1, high, args->numThreads - args_l.numThreads};
        pthread_t child_l, child_r;
        // printf("Two threads are going to be created\n");
        pthread_create(&child_l, NULL, quick_sort, (void*)&args_l); // Assign the left of the pivot index to one thread
        pthread_create(&child_r, NULL, quick_sort, (void*)&args_r); // Assign the right of the pivot index to another thread

        // printf("Im thread number: %d\n", args->numThreads);
        pthread_join(child_l, NULL);
        pthread_join(child_r, NULL);
    }
    else // No more threads allowed, continue sequentially
    {
        args_l = (struct threadArgs){low, pivot_index - 1, 0};
        args_r = (struct threadArgs){pivot_index + 1, high, 0};
        quick_sort((void *)&args_l);
        quick_sort((void *)&args_r);
    }

    return NULL;
}

void
sorted(int *v)
{
    int a = 1, d = 1, i;

    while((a == 1 || d == 1) && i < MAX_ITEMS - 1) {
        if (v[i] < v[i+1])
            d = 0;
        else if (v[i] > v[i+1])
            a = 0;
        i++;
    }

    if (a == 1)
        printf("The array is sorted in ascending order.\n");
    else if (d == 1)
        printf("The array is sorted in descending order.\n");
    else
        printf("The array is not sorted.\n");
}

int
main(int argc, char **argv)
{
    init_array();
    // print_array();

    struct timespec start_time, end_time;
    double elapsed_time;
    struct threadArgs args = {0, MAX_ITEMS - 1, THREADS}; // main thread
    pthread_t main;

    // pthread_mutex_init(&lock, NULL);

    clock_gettime(CLOCK_MONOTONIC, &start_time); // Capture start time

    pthread_create(&main, NULL, quick_sort, (void*)&args);
    pthread_join(main, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end_time); // Capture end time
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1e9; // Calculate elapsed time


    printf("Time taken for parallel with threads %d: %.3f seconds\n", THREADS, elapsed_time);

    // sorted(v);
    // print_array();

    free(v);
    // pthread_mutex_destroy(&lock);
    return 0;
}
