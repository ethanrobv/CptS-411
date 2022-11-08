// serial implementation of a simple pseudo-random number generator
// begins with a seed phrase and each subsequent number is generated performing
// a matrix multiplication on a vector comprised of the previous number and 1, and
// a base matrix with some user-defined constants


#include <stdio.h>
#include <stdlib.h>

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
    if (argc != 6)
    {
        printf("usage: rng_serial N A B P seed\n");
        return -1;
    }

    int N = atoi(argv[1]);
    int A = atoi(argv[2]);
    int B = atoi(argv[3]);
    int P = atoi(argv[4]);
    int seed = atoi(argv[5]);

    int* array = (int*)malloc(sizeof(int)*N);

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

    for (int i = 0; i < N; i++)
    {
        if (i == 0)
        {
            array[i] = seed;
        }
        else
        {
            int** M2 = (int**)malloc(sizeof(int*)*2);
            for (int i = 0; i < 2; i++)
            {
                M2[i] = (int*)malloc(sizeof(int)*1);
            }
            M2[0][0] = array[i-1];
            M2[1][0] = 1;

            int** result = modified_matrix_multiply(M0, M2, 2, 2, 2, 1, P);
            int** result2 = modified_matrix_multiply(M1, result, 2, 2, 2, 1, P);
            array[i] = result2[0][0];
        }
    }

    printf("array: ");
    for (int i = 0; i < N; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");

    //free memory
    for (int i = 0; i < 2; i++)
    {
        free(M0[i]);
        free(M1[i]);
    }
    free(M0);
    free(M1);
    free(array);

    return 0;
}