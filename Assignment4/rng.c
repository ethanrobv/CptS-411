/*
    Parallel prefix: random number generation
*/

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define __DEBUG__ 1

const int N = 1024;
const int A = 5;
const int B = 7;
const int P = 74609;
const int seed = 123;

// multiply M1 x M2 while mod the inner product by P
int** modified_matrix_multiply(int** M1, int** M2, int M1_rows, int M1_cols, int M2_rows, int M2_cols)
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

    // locally compute M^(n/p) at each rank
    int** Mnp = (int**)malloc(sizeof(int*)*2);
    for (int i = 0; i < 2; i++)
    {
        Mnp[i] = (int*)malloc(sizeof(int)*2);
    }
    for (int i = 0; i <= N/p; i++)
    {
        if (i == 0)
        {
            Mnp = M0;
        }
        else
        {
            Mnp = modified_matrix_multiply(Mnp, M1, 2, 2, 2, 2);
        }
    }

    if (__DEBUG__)
    {
        printf("****Proc %d****\nM^(n/p)=[[%d, %d]\n         [%d, %d]]\n", rank, Mnp[0][0], Mnp[0][1], Mnp[1][0], Mnp[1][1]);
    }

    // compute M(rank * n/p) at each rank
    int** Mranknp = (int**)malloc(sizeof(int*)*2);
    for (int i = 0; i < 2; i++)
    {
        Mranknp[i] = (int*)malloc(sizeof(int)*2);
    }
    for (int i = 0; i <= rank*N/p; i++)
    {
        if (i == 0)
        {
            Mranknp = M0;
        }
        else
        {
            Mranknp = modified_matrix_multiply(Mranknp, M1, 2, 2, 2, 2);
        }
    }

    if (__DEBUG__)
    {
        printf("****Proc %d****\nM(rank * n/p)=[[%d, %d]\n               [%d, %d]]\n", rank, Mranknp[0][0], Mranknp[0][1], Mranknp[1][0], Mranknp[1][1]);
    }

    for (; iterator < N/p; iterator++)
    {
        partial_array[iterator] = (Mranknp[0][0]*seed + Mranknp[0][1])%P;
        Mranknp = modified_matrix_multiply(Mranknp, M1, 2, 2, 2, 2);
    }

    // gather partial arrays
    int* array = (int*)malloc(sizeof(int)*N);
    MPI_Gather(partial_array, N/p, MPI_INT, array, N/p, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
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
        free(Mnp[i]);
        free(Mranknp[i]);
    }
    free(M0);
    free(M1);
    free(Mnp);
    free(Mranknp);


    MPI_Finalize();

    return 0;
}