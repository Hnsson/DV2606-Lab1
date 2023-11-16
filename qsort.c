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
// #define MAX_ITEMS 24
#define MAX_ITEMS (64*MEGA)
#define MAX_THREADS 8 // Adjust this value to limit the maximum number of threads
#define THRESHOLD MAX_ITEMS/MAX_THREADS
#define swap(v, a, b) {unsigned tmp; tmp=v[a]; v[a]=v[b]; v[b]=tmp;}

static int *v;
pthread_t pt[MAX_THREADS];

static unsigned partition(int *v, unsigned low, unsigned high, unsigned pivot_index);
void *child(void *params);
static void quick_sort(int *v, unsigned low, unsigned high);

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

static void
quick_sort(int *v, unsigned low, unsigned high)
{
    unsigned pivot_index;

    if (low >= high) {
        return;
    }

    pivot_index = (low + high) / 2;
    pivot_index = partition(v, low, high, pivot_index);

    if (low < pivot_index) {
        quick_sort(v, low, pivot_index - 1);
    }
    if (pivot_index < high) {
        quick_sort(v, pivot_index + 1, high);
    }
}

void *child(void *params) {
    unsigned long i = (unsigned long) params;

    unsigned pivot_index;
    int low = 0;
    int high = MAX_ITEMS;

    if (low >= high)
        return NULL;

    pivot_index = (low + high) / 2;
    pivot_index = partition(v, low, high, pivot_index);

    if (((pivot_index - low) <= THRESHOLD) || (i == MAX_THREADS - 1)) {
        quick_sort(v, low, pivot_index);
    }
    else if(i < MAX_THREADS) {
        ++i;
        pthread_create(&pt[i], NULL, *child, (void*)(__intptr_t)i);
        pthread_join(pt[i], NULL);
    }

    if(((high - pivot_index) <= THRESHOLD) || (i == MAX_THREADS - 1)) {
        quick_sort(v, pivot_index, high);
    }
    else if (i < MAX_THREADS) {
        ++i;
        pthread_create(&pt[i], NULL, *child, (void*)(__intptr_t)i);
        pthread_join(pt[i], NULL);
    }

    return NULL;
}

int main(int argc, char **argv) {
    // pt = malloc(MAX_THREADS * sizeof(pthread_t)); 
    init_array();

    // printf("UNSORTED ARRAY:\n");
    // print_array();
    // printf("\n\n");

    int i = 0;
    pthread_create(&pt[i], NULL, *child, (void*)(__intptr_t)i);
    pthread_join(pt[i], NULL);

    // print_array();

    free(v);
    return 0;
}