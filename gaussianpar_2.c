/***************************************************************************
 *
 * Sequential version of Gaussian elimination
 *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_SIZE 4096
#define THREADS 512
pthread_barrier_t barrier;
pthread_barrier_t p1_barrier;

typedef double matrix[MAX_SIZE][MAX_SIZE];

int	N;		/* matrix size		*/
int	maxnum;		/* max number of element*/
char	*Init;		/* matrix init type	*/
int	PRINT;		/* print switch		*/
matrix	A;		/* matrix A		*/
double	b[MAX_SIZE];	/* vector b             */
double	y[MAX_SIZE];	/* vector y             */

/* forward declarations */
void *work(void* params);
void Init_Matrix(void);
void Print_Matrix(void);
void Init_Default(void);
void Init_Work(void);
int Read_Options(int, char **);

int
main(int argc, char **argv)
{
    int i, timestart, timeend, iter;

    Init_Default();		/* Init default values	*/
    Read_Options(argc,argv);	/* Read arguments	*/
    Init_Matrix();		/* Init the matrix	*/

    Init_Work();
    // work();
    if (PRINT == 1)
	   Print_Matrix();
}

void
Init_Work(void)
{
    pthread_t *children;
    unsigned int numThreads = THREADS;
    numThreads = numThreads > 0 ? numThreads : 1;
    children = malloc(numThreads * sizeof(pthread_t));

    pthread_barrier_init(&barrier, NULL, THREADS);
    pthread_barrier_init(&p1_barrier, NULL, THREADS+1);

    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_create(&(children[id]),
            NULL,
            work,
            (void*)(__intptr_t)id
        );
    }

    pthread_barrier_wait(&p1_barrier);

    int i, j, k;

    for (k = 0; k < N; k++) {
        y[k] = b[k]/A[k][k];
        A[k][k] = 1;
        for (i = k + 1; i < N; i++) {
            b[i] = b[i] - A[i][k] * y[k];
            A[i][k] = 0;
        }
    }



    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_join(children[id], NULL);
    }
    
    pthread_barrier_destroy(&barrier);
    pthread_barrier_destroy(&p1_barrier);
	free(children); // deallocate array
}


void
*work(void* params)
{
    unsigned long id = (unsigned long) params;
    int i, j, k;

    for (k = 0; k < N; k++) {
        // Wait for all threads to synchronize before proceeding
        pthread_barrier_wait(&barrier);
        for (j = k + 1 + id; j < N; j += THREADS) {
            A[k][j] = A[k][j] / A[k][k];
            for (i = k + 1; i < N; i++) {
                A[i][j] = A[i][j] - A[i][k] * A[k][j];
            }
        }
    }
    // Wait for all threads to finish before moving to the next iteration
    pthread_barrier_wait(&p1_barrier);

    return NULL;
}

void
Init_Matrix()
{
    int i, j;

    printf("\nthreads   = %d", THREADS);
    printf("\nsize      = %dx%d ", N, N);
    printf("\nmaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...");

    if (strcmp(Init,"rand") == 0) {
        for (i = 0; i < N; i++){
            for (j = 0; j < N; j++) {
                if (i == j) /* diagonal dominance */
                    A[i][j] = (double)(rand() % maxnum) + 5.0;
                else
                    A[i][j] = (double)(rand() % maxnum) + 1.0;
            }
        }
    }
    if (strcmp(Init,"fast") == 0) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                if (i == j) /* diagonal dominance */
                    A[i][j] = 5.0;
                else
                    A[i][j] = 2.0;
            }
        }
    }

    /* Initialize vectors b and y */
    for (i = 0; i < N; i++) {
        b[i] = 2.0;
        y[i] = 1.0;
    }

    printf("done \n\n");
    if (PRINT == 1)
        Print_Matrix();
}

void
Print_Matrix()
{
    int i, j;

    printf("Matrix A:\n");
    for (i = 0; i < N; i++) {
        printf("[");
        for (j = 0; j < N; j++)
            printf(" %5.2f,", A[i][j]);
        printf("]\n");
    }
    printf("Vector b:\n[");
    for (j = 0; j < N; j++)
        printf(" %5.2f,", b[j]);
    printf("]\n");
    printf("Vector y:\n[");
    for (j = 0; j < N; j++)
        printf(" %5.2f,", y[j]);
    printf("]\n");
    printf("\n\n");
}

void
Init_Default()
{
    N = 16;
    Init = "rand";
    maxnum = 15.0;
    PRINT = 1;
}

int
Read_Options(int argc, char **argv)
{
    char    *prog;

    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch ( *++*argv ) {
                case 'n':
                    --argc;
                    N = atoi(*++argv);
                    break;
                case 'h':
                    printf("\nHELP: try sor -u \n\n");
                    exit(0);
                    break;
                case 'u':
                    printf("\nUsage: gaussian [-n problemsize]\n");
                    printf("           [-D] show default values \n");
                    printf("           [-h] help \n");
                    printf("           [-I init_type] fast/rand \n");
                    printf("           [-m maxnum] max random no \n");
                    printf("           [-P print_switch] 0/1 \n");
                    exit(0);
                    break;
                case 'D':
                    printf("\nDefault:  n         = %d ", N);
                    printf("\n          Init      = rand" );
                    printf("\n          maxnum    = 5 ");
                    printf("\n          P         = 0 \n\n");
                    exit(0);
                    break;
                case 'I':
                    --argc;
                    Init = *++argv;
                    break;
                case 'm':
                    --argc;
                    maxnum = atoi(*++argv);
                    break;
                case 'P':
                    --argc;
                    PRINT = atoi(*++argv);
                    break;
                default:
                    printf("%s: ignored option: -%s\n", prog, *argv);
                    printf("HELP: try %s -u \n\n", prog);
                    break;
            }
}
