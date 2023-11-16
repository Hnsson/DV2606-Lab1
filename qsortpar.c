/***************************************************************************
 *
 * Parallel version of Quick sort
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define KILO (1024)
#define MEGA (1024*1024)
// #define MAX_ITEMS 8
#define MAX_ITEMS (64*MEGA)
#define MAX_THREADS 8 // Adjust this value to limit the maximum number of threads
#define THRESHOLD MAX_ITEMS/MAX_THREADS
#define swap(v, a, b) {unsigned tmp; tmp=v[a]; v[a]=v[b]; v[b]=tmp;}

static int *v;

struct threadArgs {
    int low;
    int high;
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
        v[i] = rand();
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

void *quick_sort(void *params) {
    struct threadArgs *args = (struct threadArgs *)params;

    unsigned pivot_index;
    int low = args->low;
    int high = args->high;

    if (low >= high)
        return NULL;

    pivot_index = (low + high) / 2;
    pivot_index = partition(v, low, high, pivot_index);

    pthread_t left_thread, right_thread;
    struct threadArgs left_args, right_args;

    if (low < pivot_index) {
        left_args.low = low;
        left_args.high = pivot_index;

        if ((pivot_index - low) > MAX_ITEMS / MAX_THREADS) {
            pthread_create(&left_thread, NULL, quick_sort, (void *)&left_args);
            pthread_join(left_thread, NULL);
        } else {
            quick_sort(&left_args);
        }
    }

    if (pivot_index < high) {
        right_args.low = pivot_index + 1;
        right_args.high = high;

        if ((high - (pivot_index + 1)) > MAX_ITEMS / MAX_THREADS) {
            pthread_create(&right_thread, NULL, quick_sort, (void *)&right_args);
            pthread_join(right_thread, NULL);
        } else {
            quick_sort(&right_args);
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    init_array();

    // print_array();

    struct threadArgs args;
    args.low = 0;
    args.high = MAX_ITEMS - 1;
    quick_sort(&args);
    
    print_array();

    free(v);
    return 0;
}