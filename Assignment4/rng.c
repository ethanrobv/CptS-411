/*
    Parallel prefix: random number generation
*/

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define __DEBUG__ 0

// multiply M1 x M2 while mod the inner product by P
int** modified_matrix_multiply(int** M1, int** M2, int M1_rows, int M1_cols, int M2_rows, int M2_cols, int P)
{
    int** result = (int**)malloc(sizeof(int*)*M1_cols);
    for (int i = 0; i < M2_rows; i++)
    {
        result[i] = (int*)malloc(sizeof(int)*M2_cols);
    }

    // compute dot product and mod them by P
    for (int i = 0; i < M1_rows; i++)
    {
        for (int j = 0; j < M2_cols; j++)
        {
            int sum = 0;
            for (int k = 0; k < M1_cols; k++)
            {
                sum += M1[i][k]*M2[k][j];
            }
            result[i][j] = sum%P;
        }
    }

    return result;
}


int main(int argc, char** argv)
{
    int rank, p;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc != 6)
    {
        printf("Usage: ./run_ParallelRNG.sh N A B P seed numProcs\n");
        return -1;
    }

    int N = atoi(argv[1]);
    int A = atoi(argv[2]);
    int B = atoi(argv[3]);
    int P = atoi(argv[4]);
    int seed = atoi(argv[5]);

    double start_time, end_time;
    start_time = MPI_Wtime();

    // init partial array
    int* partial_array = (int*)malloc(sizeof(int)*N/p);
    int iterator = 0;

    // init basis matrixes
    int** M0 = (int**)malloc(sizeof(int*)*2);
    for (int i = 0; i < 2; i++)
    {
        M0[i] = (int*)malloc(sizeof(int)*2);
    }
    M0[0][0] = 1;
    M0[0][1] = 0;
    M0[1][0] = 0;
    M0[1][1] = 1;

    int** M1 = (int**)malloc(sizeof(int*)*2);
    for (int i = 0; i < 2; i++)
    {
        M1[i] = (int*)malloc(sizeof(int)*2);
    }
    M1[0][0] = A;
    M1[0][1] = 0;
    M1[1][0] = B;
    M1[1][1] = 1;

    // compute M(rank * n/p) at each rank
    int** Mranknp = (int**)malloc(sizeof(int*)*2);
    for (int i = 0; i < 2; i++)
    {
        Mranknp[i] = (int*)malloc(sizeof(int)*2);
    }
    for (int i = 0; i <= rank*N/p; i++)
    {
        if (__DEBUG__)
            printf("proc %d: computing M(%d)\n", rank, i);
        if (i == 0)
        {
            Mranknp = M0;
        }
        else
        {
            Mranknp = modified_matrix_multiply(Mranknp, M1, 2, 2, 2, 2, P);
        }
    }

    if (__DEBUG__)
    {
        printf("****Proc %d****\nM(rank * n/p)=[[%d, %d]\n               [%d, %d]]\n", rank, Mranknp[0][0], Mranknp[0][1], Mranknp[1][0], Mranknp[1][1]);
    }

    int* prefix_vector = (int*)malloc(sizeof(int)*2);
    prefix_vector[0] = seed;
    prefix_vector[1] = 1;

    for (; iterator < N/p; iterator++)
    {
        prefix_vector = *(modified_matrix_multiply(&prefix_vector, Mranknp, 1, 2, 2, 2, P));
        partial_array[iterator] = prefix_vector[0];
        Mranknp = modified_matrix_multiply(Mranknp, M1, 2, 2, 2, 2, P);
        prefix_vector[0] = seed;
        prefix_vector[1] = 1;
    }

    // gather partial arrays
    int* array = (int*)malloc(sizeof(int)*N);
    MPI_Gather(partial_array, N/p, MPI_INT, array, N/p, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0 && __DEBUG__)
    {
        printf("Array: ");
        for (int i = 0; i < N; i++)
        {
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    // free memory
    free(partial_array);
    free(array);
    for (int i = 0; i < 2; i++)
    {
        free(M0[i]);
        free(M1[i]);
        free(Mranknp[i]);
    }
    free(M0);
    free(M1);
    free(Mranknp);

    end_time = MPI_Wtime();
    MPI_Allreduce(MPI_IN_PLACE, &end_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    if (rank == 0)
    {
        printf("Time elapsed in microseconds (time of longest proc): %lf\n", (end_time - start_time)*1000000);
    }


    MPI_Finalize();

    return 0;
}