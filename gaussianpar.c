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
    unsigned int numThreads = 16;
    numThreads = numThreads > 0 ? numThreads : 1;
    children = malloc(numThreads * sizeof(pthread_t));

    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_create(&(children[id]),
            NULL,
            work,
            (void*)(__intptr_t)id
        );
    }

    for (unsigned int id = 0; id < numThreads; id++) {
        pthread_join(children[id], NULL);
    }

	free(children); // deallocate array
}


void
*work(void* params)
{
    unsigned long id = (unsigned long) params;
    int i, j, k;

    unsigned int numThreads = N / 16;
    numThreads = numThreads > 0 ? numThreads : 1;

    /* Gaussian elimination algorithm, Algo 8.4 from Grama */
    for (k = id*(N/numThreads); k < (id + 1)*(N/numThreads); k++) { /* Outer loop */
	    for (j = k+1; j < N; j++)
	       A[k][j] = A[k][j] / A[k][k]; /* Division step */
	    y[k] = b[k] / A[k][k];
	    A[k][k] = 1.0;
	    for (i = k+1; i < N; i++) {
	        for (j = k+1; j < N; j++)
		        A[i][j] = A[i][j] - A[i][k]*A[k][j]; /* Elimination step */
	        b[i] = b[i] - A[i][k]*y[k];
	        A[i][k] = 0.0;
	    }
    }
}

void
Init_Matrix()
{
    int i, j;

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
    N = 2048;
    Init = "rand";
    maxnum = 15.0;
    PRINT = 0;
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


/*
GAUSSIAN SEQUENTIAL RUN:
size      = 2048x2048 
maxnum    = 15 
Init      = rand 
Initializing matrix...done 


real    0m5.491s
user    0m5.490s
sys     0m0.000s

GAUSSIAN PARALLEL RUN:
size      = 2048x2048 
maxnum    = 15 
Init      = rand 
Initializing matrix...done 


real    0m0.980s
user    0m13.538s
sys     0m0.039s
*/