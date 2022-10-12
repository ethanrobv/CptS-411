/*
* Ethan Vincent
* Parallel implementation of Conway's Game of Life
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mpi.h>

#define HEIGHT 16
#define WIDTH 16
# define NUM_ITERATIONS 10

void GenerateInitialGOL(int partial_board[][WIDTH], int rank, int p)
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
}

void PrintBoard(int board[][WIDTH], int rank, int p)
{
    // every proc sends their local board to p0
    if (rank != 0)
    {
        MPI_Send(&board[0][0], (HEIGHT/p)*WIDTH, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        // p0 prints its local board
        for (int i = 0; i < (HEIGHT/p); i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                printf("%d ", board[i][j]);
            }
            printf("\n");
        }

        // p0 receives and prints the boards of the other procs
        for (int i = 1; i < p; i++)
        {
            int recv_board[(HEIGHT/p)][WIDTH];
            MPI_Recv(recv_board, (HEIGHT/p)*WIDTH, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j < (HEIGHT/p); j++)
            {
                for (int k = 0; k < WIDTH; k++)
                {
                    printf("%d ", recv_board[j][k]);
                }
                printf("\n");
            }
        }
    }
}

int DetermineState(int x, int y, int partial_board[][WIDTH], int top_row[WIDTH], int bottom_row[WIDTH], int rank, int p)
{
    // visit all 8 neighbors to determine next state
    int neighbor_sum = 0;
    
    // ADD FIRST NEIGHBOR: TOP LEFT CORNER
    // if x == 0 and y==0, we grab from right most cell in top_row
    if (x == 0 && y == 0)
    {
        neighbor_sum += top_row[WIDTH-1];
    }
    else if (x == 0)
    {
        neighbor_sum += top_row[y-1];
    }
    else
    {
        neighbor_sum += partial_board[x-1][y-1];
    }

    // ADD SECOND NEIGHBOR: TOP MIDDLE
    if (x == 0)
    {
        neighbor_sum += top_row[y];
    }
    else
    {
        neighbor_sum += partial_board[x-1][y];
    }

    // ADD THIRD NEIGHBOR: TOP RIGHT CORNER
    // if x == 0 and y == WIDTH-1, we grab from left most cell in top_row
    if (x == 0 && y == WIDTH-1)
    {
        neighbor_sum += top_row[0];
    }
    else if (x == 0)
    {
        neighbor_sum += top_row[y+1];
    }
    else
    {
        neighbor_sum += partial_board[x-1][y+1];
    }

    // ADD FOURTH NEIGHBOR: LEFT MIDDLE
    if (y == 0)
    {
        neighbor_sum += partial_board[x][WIDTH-1];
    }
    else
    {
        neighbor_sum += partial_board[x][y-1];
    }

    // ADD FIFTH NEIGHBOR: RIGHT MIDDLE
    if (y == WIDTH-1)
    {
        neighbor_sum += partial_board[x][0];
    }
    else
    {
        neighbor_sum += partial_board[x][y+1];
    }

    // ADD SIXTH NEIGHBOR: BOTTOM LEFT CORNER
    // if x == (HEIGHT/p)-1 and y == 0, we grab from right most cell in bottom_row
    if (x == (HEIGHT/p)-1 && y == 0)
    {
        neighbor_sum += bottom_row[WIDTH-1];
    }
    else if (x == (HEIGHT/p)-1)
    {
        neighbor_sum += bottom_row[y-1];
    }
    else
    {
        neighbor_sum += partial_board[x+1][y-1];
    }

    // ADD SEVENTH NEIGHBOR: BOTTOM MIDDLE
    if (x == (HEIGHT/p)-1)
    {
        neighbor_sum += bottom_row[y];
    }
    else
    {
        neighbor_sum += partial_board[x+1][y];
    }

    // ADD EIGHTH NEIGHBOR: BOTTOM RIGHT CORNER
    // if x == (HEIGHT/p)-1 and y == WIDTH-1, we grab from left most cell in bottom_row
    if (x == (HEIGHT/p)-1 && y == WIDTH-1)
    {
        neighbor_sum += bottom_row[0];
    }
    else if (x == (HEIGHT/p)-1)
    {
        neighbor_sum += bottom_row[y+1];
    }
    else
    {
        neighbor_sum += partial_board[x+1][y+1];
    }
   
    // determine next state
    if (neighbor_sum > 2 && neighbor_sum < 6)
    {
        return 1;
    }

    return 0;
}

void Simulate(int partial_board[][WIDTH], int rank, int p, int num_iterations)
{
    // each iteration, send the top and bottom rows to the appropriate proc
    int top_row[WIDTH];
    int bottom_row[WIDTH];
    
    for (int i = 0; i < num_iterations; i++)
    {
        // MPI_Barrier to synchronize all procs
        MPI_Barrier(MPI_COMM_WORLD);

        // copy top and bottom rows
        for (int i = 0; i < WIDTH; i++)
        {
            top_row[i] = partial_board[0][i];
            bottom_row[i] = partial_board[(HEIGHT/p)-1][i];
        }

        /*
        * MPI Sends and Receives
        */
        if (rank == 0)
        {
            // p0 top row is p-1 bottom row
            MPI_Send(top_row, WIDTH, MPI_INT, p-1, 2, MPI_COMM_WORLD);

            MPI_Send(bottom_row, WIDTH, MPI_INT, rank+1, 1, MPI_COMM_WORLD);

            // p-1 bottom row is p0 top row
            MPI_Recv(top_row, WIDTH, MPI_INT, p-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Recv(bottom_row, WIDTH, MPI_INT, rank+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else if (rank == p-1)
        {
            // 0 top row is p-1 bottom row
            MPI_Send(bottom_row, WIDTH, MPI_INT, 0, 1, MPI_COMM_WORLD);

            MPI_Send(top_row, WIDTH, MPI_INT, rank-1, 2, MPI_COMM_WORLD);

            // 0 top row is p-1 bottom row
            MPI_Recv(bottom_row, WIDTH, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Recv(top_row, WIDTH, MPI_INT, rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else
        {
            // send top row to proc above
            MPI_Send(top_row, WIDTH, MPI_INT, rank-1, 2, MPI_COMM_WORLD);
            // send bottom row to proc below
            MPI_Send(bottom_row, WIDTH, MPI_INT, rank+1, 1, MPI_COMM_WORLD);

            // receive bottom row from proc above
            MPI_Recv(top_row, WIDTH, MPI_INT, rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // receive top row from proc below
            MPI_Recv(bottom_row, WIDTH, MPI_INT, rank+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        /*
        * Determine new state
        */

        int new_board[(HEIGHT/p)][WIDTH];
        for (int x = 0; x < HEIGHT/p; x++)
        {
            for (int y = 0; y < WIDTH; y++)
            {
                new_board[x][y] = DetermineNextState(partial_board, x, y, top_row, bottom_row, rank, p);
            }
        }

        // copy new_board to partial_board
        for (int x = 0; x < HEIGHT/p; x++)
        {
            for (int y = 0; y < WIDTH; y++)
            {
                partial_board[x][y] = new_board[x][y];
            }
        }

        if (i % 2 == 0)
        {
            PrintBoard(partial_board, rank, p);
        }
    }
}

int main(int argc, char** argv)
{
    int rank,p;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int partial_board[(HEIGHT/p)][WIDTH];
    //printf("Proc %d: line 420\n", rank);
    GenerateInitialGOL(partial_board, rank, p);
    //printf("Proc %d: line 422\n", rank);
    //PrintBoard(partial_board, rank, p);

    Simulate(partial_board, rank, p, NUM_ITERATIONS);

    MPI_Finalize();
    return 0;
}