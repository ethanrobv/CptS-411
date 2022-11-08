// serial implementation of a simple pseudo-random number generator
// begins with a seed phrase and each subsequent number is generated performing
// a matrix multiplication on a vector comprised of the previous number and 1, and
// a base matrix with some user-defined constants


#include <stdio.h>
#include <stdlib.h>

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


    printf("array: ");
    for (int i = 0; i < N; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");

    //free memory
    free(array);

    return 0;
}