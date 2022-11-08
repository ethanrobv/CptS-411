// serial implementation of a simple pseudo-random number generator
// begins with a seed phrase and each subsequent number is generated performing
// a matrix multiplication on a vector comprised of the previous number and 1, and
// a base matrix with some user-defined constants

#define __DEBUG__ 0

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>


int main(int argc, char** argv)
{
    struct timezone tz;
    struct timeval tv;
    double start, end;


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

    gettimeofday(&tv, NULL);
    start = (double)tv.tv_sec / (1000000) + (double)tv.tv_usec;


    int* array = (int*)malloc(sizeof(int)*N);

   // recurrence relation: x_i = {Ax_i-1 + B} mod P, seed if i = 0}
   for (int i = 0; i < N; i++)
   {
        if (i == 0)
        {
            array[i] = seed;
        }
        else
        {
            array[i] = (A*array[i-1] + B)%P;
        }
   }

    if (__DEBUG__)
    {
        printf("array: ");
        for (int i = 0; i < N; i++)
        {
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    //free memory
    free(array);

    gettimeofday(&tv, NULL);
    end = (double)tv.tv_sec / (1000000) + (double)tv.tv_usec;

    //get elapsed time in microseconds
    double elapsed = end - start;

    printf("elapsed time: %lf microseconds\n", elapsed);

    return 0;
}