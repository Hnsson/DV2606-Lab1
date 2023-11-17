#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREADS 4
#define KILO (1024)
#define MEGA (1024*1024) //1 048 576
#define MAX_ITEMS (64 *MEGA) //67 108 864
// #define MAX_ITEMS 16
#define swap(v, a, b) {unsigned tmp; tmp=v[a]; v[a]=v[b]; v[b]=tmp;}

// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int *v; 
// int global_thread = 0; //Starts at 1, because the thread from main is the first

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

static int partition(int *v, int low, int high, int pivot_index)
{
    if (pivot_index != low)
        swap(v, low, pivot_index);

    pivot_index = low;
    low++;

    while (low <= high) 
    {
        if (v[low] <= v[pivot_index]) 
            low++;
        else if (v[high] > v[pivot_index])
            high--;
        else
            swap(v, low, high);
    }
    
    if (high != pivot_index)
        swap(v, pivot_index, high);

    return high;
}

void* quick_sort(void* arguments)
{
    struct threadArgs* parent = (struct threadArgs*) arguments;
    
    int pivot_index;

    int low = parent->low;
    int high = parent->high;
    int numThreads = parent->numThreads;

    if (low >= high)
        return NULL;

    pivot_index = (low + high) / 2;
    pivot_index = partition(v, low, high, pivot_index);

    struct threadArgs child_thread_1;
    child_thread_1.low = low;
    child_thread_1.high = pivot_index - 1;
    child_thread_1.numThreads = numThreads / 2;

    struct threadArgs child_thread_2;
    child_thread_2.low = pivot_index +1;
    child_thread_2.high = high;
    child_thread_2.numThreads = numThreads - child_thread_1.numThreads;
    
    if(numThreads > 1) //should only create 7 new threads, 1 is already from main
    {
        // parent->low = low;
        // parent->high = pivot_index - 1;
        // parent->numThreads = numThreads / 2;
        // quick_sort((void*) parent);
        pthread_t child_1;
        
        pthread_create(&child_1, NULL, quick_sort, (void*)&child_thread_1);

        pthread_t child_2;
        
        pthread_create(&child_2, NULL, quick_sort, (void*)&child_thread_2);


        pthread_join(child_1, NULL);
        pthread_join(child_2, NULL);

        // if(low < pivot_index) //parent: left side,
        // {
        //     parent->low = low;
        //     parent->high = pivot_index - 1;
        //     quick_sort((void*) parent);
        // }
        // if(pivot_index < high) //child: right side
        // {
        //     pthread_t child;
    
        //     struct threadArgs child_thread;
        //     child_thread.low = pivot_index +1;
        //     child_thread.high = high;
        //     global_thread++;
            
        //     pthread_create(&child, NULL, quick_sort, (void*)&child_thread);
        //     pthread_join(child, NULL);
        //     // free(child_thread);
        // }
    }
    else
    {
            // parent->low = low;
            // parent->high = pivot_index - 1;
            // quick_sort((void*) parent);
            quick_sort((void *)&child_thread_1);

            // parent->low = pivot_index +1;
            // parent->high = high;
            // quick_sort((void*) parent);
            quick_sort((void*) &child_thread_2);

    }
    return NULL;
}

void
sorted(int *v) {
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

int main(int argc, char **argv)
{
    init_array();
    // print_array();

    struct threadArgs args; // main thread
    args.low = 0;
    args.high = MAX_ITEMS - 1;
    args.numThreads = THREADS;
    // args =  malloc(sizeof(struct threadArgs));
    // args->low = 0;
    // args->high = MAX_ITEMS - 1;
    pthread_t main;

    pthread_create(&main, NULL, quick_sort, (void*)&args);
    pthread_join(main, NULL);

    // quick_sort((void *)args);
    // free(v);

    // sorted(v);
    // printf("%i Threads were created", global_thread);

    // print_array();
    // free(v);
    return 0;
}
