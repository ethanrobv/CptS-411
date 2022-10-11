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

int GenerateInitialGOL(int** partial_board, int rank, int p)
{
    

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

void print_board(int** board, int rank, int p)
{
    if (board == NULL)
    {
        printf("Board is NULL.\n");
        return;
    }
    
    // each proc sends their partial board to p0
    if (rank != 0)
    {
        MPI_Send(board, HEIGHT/p*WIDTH, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        // p0 prints its own partial board
        for (int i = 0; i < HEIGHT/p; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                printf("%d ", board[i][j]);
            }
            printf("\n");
        }

        // p0 receives and prints the partial boards of the other procs
        for (int i = 1; i < p; i++)
        {
            int** recv_board = (int**)malloc(HEIGHT/p*sizeof(int*));
            for (int j = 0; j < HEIGHT/p; j++)
            {
                recv_board[j] = (int*)malloc(WIDTH*sizeof(int));
            }
            MPI_Recv(recv_board, HEIGHT/p*WIDTH, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j < HEIGHT/p; j++)
            {
                for (int k = 0; k < WIDTH; k++)
                {
                    if (recv_board[j][k] == Dead)
                    {
                        printf("-  ");
                    }
                    else
                    {
                        printf("O ");
                    }
                    //printf("%d ", recv_board[j][k]);
                }
                printf("\n");
            }

            for (int j = 0; j < HEIGHT/p; j++)
            {
                free(recv_board[j]);
            }
            free (recv_board);
        }
    }
}

int main(int argc, char** argv)
{
    int rank,p;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int** partial_board = (int**)malloc(HEIGHT/p*sizeof(int*));
    for (int i = 0; i < WIDTH; i++)
    {
        partial_board[i] = (int*)malloc(WIDTH*sizeof(int));
    }

    GenerateInitialGOL(partial_board, rank, p);
    MPI_Barrier(MPI_COMM_WORLD);
    print_board(partial_board, rank, p);


    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < HEIGHT/p; i++)
    {
        free(partial_board[i]);
    }
    free(partial_board);

    MPI_Finalize();
    return 0;
}