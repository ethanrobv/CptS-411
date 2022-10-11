/*
* Ethan Vincent
* Parallel implementation of Conway's Game of Life
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mpi.h>

#define HEIGHT 32
#define WIDTH 32

enum State {Dead = 0, Alive = 1};

int* GenerateInitialGOL(int rank, int p)
{
    int (*partial_board)[WIDTH] = malloc(sizeof(int[HEIGHT/p][WIDTH]));

    // give each process a random seed (except p0, which uses system time)
    if (rank == 0)
    {
        srand(time(NULL));
        for (int i = 1; i < p; i++)
        {
            int seed = rand() % (217645199) + 1;
            MPI_Send(&seed, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        int my_seed = rand() % (217645199) + 1;
    }
    else
    {
        int recv_seed = 0;
        MPI_Recv(&recv_seed, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        srand(recv_seed);
    }

    // generate random values to determine cell state
    for (int i = 0; i < (HEIGHT/p); i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            partial_board[i][j] = rand() % 2;
        }
    }

    return partial_board;
}

int main(int argc, char** argv)
{
    int rank,p;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int* partial_board = GenerateInitialGOL(rank, p);

    free (partial_board);
    MPI_Finalize();
    return 0;
}