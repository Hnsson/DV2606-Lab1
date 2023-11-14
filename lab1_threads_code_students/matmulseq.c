/***************************************************************************
 *
 * Sequential version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 2048
#define NR_THREADS 128


static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

void
*matmul_init(void* params) {
    unsigned long id = (unsigned long) params;
    int j;

    for (j = 0; j < SIZE; j++) {
        /* Simple initialization, which enables us to easy check
            * the correct answer. Each element in c will have the same
            * value as SIZE after the matmul operation.
            */
        a[id][j] = 1.0;
        b[id][j] = 1.0;
    }
}

static void
init_matrix_par(void) {
    pthread_t *children;
    unsigned int numThreads = SIZE;
    children = malloc(numThreads * sizeof(pthread_t));

    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_create(&(children[id]),
            NULL,
            matmul_init,
            (void*)(__intptr_t)id
        );
    }

    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_join(children[id], NULL);
    }

	free(children); // deallocate array
}

static void
init_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++) {
	        /* Simple initialization, which enables us to easy check
	         * the correct answer. Each element in c will have the same
	         * value as SIZE after the matmul operation.
	         */
	        a[i][j] = 1.0;
	        b[i][j] = 1.0;
        }
}

static void
matmul_seq()
{
    int i, j, k;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            c[i][j] = 0.0;
            for (k = 0; k < SIZE; k++)
                c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
    }
}

void
*matmul_calc_2(void* params) {
    unsigned long id = (unsigned long) params;
    int i, j, k;

    for (i = id*(SIZE/NR_THREADS); i < (id + 1)*(SIZE/NR_THREADS); i++) {
        for (j = 0; j < SIZE; j++) {
            c[i][j] = 0.0;
            for (k = 0; k < SIZE; k++)
                c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
    }
}

static void
matmul_par_2() {
    pthread_t *children;
    unsigned int numThreads = NR_THREADS;
    children = malloc(numThreads * sizeof(pthread_t));

    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_create(&(children[id]),
            NULL,
            matmul_calc_2,
            (void*)(__intptr_t)id
        );
    }

    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_join(children[id], NULL);
    }

	free(children); // deallocate array
}

void
*matmul_calc(void* params) {
    unsigned long id = (unsigned long) params;
    int j, k;

    for (j = 0; j < SIZE; j++) {
        c[id][j] = 0.0;
        for (k = 0; k < SIZE; k++)
            c[id][j] = c[id][j] + a[id][k] * b[k][j];
    }
}

static void
matmul_par() {
    pthread_t *children;
    unsigned int numThreads = SIZE;
    children = malloc(numThreads * sizeof(pthread_t));

    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_create(&(children[id]),
            NULL,
            matmul_calc,
            (void*)(__intptr_t)id
        );
    }

    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_join(children[id], NULL);
    }

	free(children); // deallocate array
}

static void
print_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++)
	        printf(" %7.2f", c[i][j]);
	    printf("\n");
    }
}

int
main(int argc, char **argv)
{
    // init_matrix();
    init_matrix_par();

    // matmul_seq();
    // matmul_par();
    matmul_par_2();
    
    // print_matrix();
}


/*
FIRST SEQUENTIAL RUN WITH SEQUENTIAL MATRIX INITIALIZATION:
real    1m37.085s
user    1m37.075s
sys     0m0.010s

FIRST SEQUENTIAL RUN WITH PARALLEL MATRIX INITIALIZATION:
real    1m39.804s
user    1m39.736s
sys     0m0.070s

FIRST PARALLEL RUN WITH SEQUENTIAL MATRIX INITIALIZATION:
real    0m10.658s
user    2m46.278s
sys     0m0.888s

FIRST PARALLEL RUN WITH PARALLEL MATRIX INITIALIZATION:
real    0m11.736s
user    3m3.876s
sys     0m1.371s

FIRST PARALLEL VERSION 2 RUN WITH SEQUENTIAL MATRIX INITIALIZATION:
real    0m11.113s
user    2m55.409s
sys     0m0.388s

FIRST PARALLEL VERSION 2 RUN WITH PARALLEL MATRIX INITIALIZATION:
real    0m11.408s
user    2m58.693s
sys     0m0.399s

SPEEDUP
S = Sequential / Parallel = 9.11
*/