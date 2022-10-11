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

void DetermineState(int partial_board[][WIDTH], int rank, int p)
{
    // send top row to p+1
    // send bottom row to p-1

    int top_row[WIDTH];
    int bottom_row[WIDTH];

    // copy top and bottom rows
    for (int i = 0; i < WIDTH; i++)
    {
        top_row[i] = partial_board[0][i];
        bottom_row[i] = partial_board[(HEIGHT/p)-1][i];
    }

    // send top row to p+1
    if (rank != p-1)
    {
        MPI_Send(top_row, WIDTH, MPI_INT, rank+1, 2, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Send(top_row, WIDTH, MPI_INT, 0, 2, MPI_COMM_WORLD);
    }

    // send bottom row to p-1
    if (rank != 0)
    {
        MPI_Send(bottom_row, WIDTH, MPI_INT, rank-1, 1, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Send(bottom_row, WIDTH, MPI_INT, p-1, 1, MPI_COMM_WORLD);
    }

    // receive top row from p-1
    if (rank != 0)
    {
        MPI_Recv(top_row, WIDTH, MPI_INT, rank-1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else
    {
        MPI_Recv(bottom_row, WIDTH, MPI_INT, p-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // receive bottom row from p+1
    if (rank != p-1)
    {
        MPI_Recv(bottom_row, WIDTH, MPI_INT, rank+1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else
    {
        MPI_Recv(top_row, WIDTH, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // calculate next state of local cells
    int next_state[(HEIGHT/p)][WIDTH];
    for (int i = 0; i < (HEIGHT/p); i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            // sum neighboring cells:
            // Less than 3 neighbors: cell dies (or comes to life if dead)
            // 3-5 neighbors: cell lives
            // More than 5 neighbors: cell dies
            int neighbor_sum = 0;

            // need use top_row
            if (i == 0)
            {
                // NORTH neighbor
                neighbor_sum += top_row[j];

                // NORTHWEST neighbor
                if (j == 0)
                {
                    neighbor_sum += top_row[WIDTH-1];
                }
                else
                {
                    neighbor_sum += top_row[j-1];
                }

                // NORTHEAST neighbor
                if (j == WIDTH-1)
                {
                    neighbor_sum += top_row[0];
                }
                else
                {
                    neighbor_sum += top_row[j+1];
                }

                // SOUTH neighbor
                if (i == (HEIGHT/p)-1)
                {
                    neighbor_sum += bottom_row[j];
                }
                else
                {
                    neighbor_sum += partial_board[i+1][j];
                }

                // SOUTHWEST neighbor
                if (i == (HEIGHT/p)-1)
                {
                    if (j == 0)
                    {
                        neighbor_sum += bottom_row[WIDTH-1];
                    }
                    else
                    {
                        neighbor_sum += bottom_row[j-1];
                    }
                }
                else
                {
                    if (j == 0)
                    {
                        neighbor_sum += partial_board[i+1][WIDTH-1];
                    }
                    else
                    {
                        neighbor_sum += partial_board[i+1][j-1];
                    }
                }

                // SOUTHEAST neighbor
                if (i == (HEIGHT/p)-1)
                {
                    if (j == WIDTH-1)
                    {
                        neighbor_sum += bottom_row[0];
                    }
                    else
                    {
                        neighbor_sum += bottom_row[j+1];
                    }
                }
                else
                {
                    if (j == WIDTH-1)
                    {
                        neighbor_sum += partial_board[i+1][0];
                    }
                    else
                    {
                        neighbor_sum += partial_board[i+1][j+1];
                    }
                }
            }
            // need to use bottom_row
            else if (i == (HEIGHT/p)-1)
            {
                // NORTH neighbor
                neighbor_sum += partial_board[i-1][j];

                // NORTHWEST neighbor
                if (j == 0)
                {
                    neighbor_sum += partial_board[i-1][WIDTH-1];
                }
                else
                {
                    neighbor_sum += partial_board[i-1][j-1];
                }

                // NORTHEAST neighbor
                if (j == WIDTH-1)
                {
                    neighbor_sum += partial_board[i-1][0];
                }
                else
                {
                    neighbor_sum += partial_board[i-1][j+1];
                }

                // SOUTH neighbor
                neighbor_sum += bottom_row[j];

                // SOUTHWEST neighbor
                if (j == 0)
                {
                    neighbor_sum += bottom_row[WIDTH-1];
                }
                else
                {
                    neighbor_sum += bottom_row[j-1];
                }

                // SOUTHEAST neighbor
                if (j == WIDTH-1)
                {
                    neighbor_sum += bottom_row[0];
                }
                else
                {
                    neighbor_sum += bottom_row[j+1];
                }

                // WEST neighbor
                if (j == 0)
                {
                    neighbor_sum += partial_board[i][WIDTH-1];
                }
                else
                {
                    neighbor_sum += partial_board[i][j-1];
                }

                // EAST neighbor
                if (j == WIDTH-1)
                {
                    neighbor_sum += partial_board[i][0];
                }
                else
                {
                    neighbor_sum += partial_board[i][j+1];
                }
            }
            else
            {
                // NORTH neighbor
                neighbor_sum += partial_board[i-1][j];

                // NORTHWEST neighbor
                if (j == 0)
                {
                    neighbor_sum += partial_board[i-1][WIDTH-1];
                }
                else
                {
                    neighbor_sum += partial_board[i-1][j-1];
                }

                // NORTHEAST neighbor
                if (j == WIDTH-1)
                {
                    neighbor_sum += partial_board[i-1][0];
                }
                else
                {
                    neighbor_sum += partial_board[i-1][j+1];
                }

                // SOUTH neighbor
                neighbor_sum += partial_board[i+1][j];

                // SOUTHWEST neighbor
                if (j == 0)
                {
                    neighbor_sum += partial_board[i+1][WIDTH-1];
                }
                else
                {
                    neighbor_sum += partial_board[i+1][j-1];
                }

                // SOUTHEAST neighbor
                if (j == WIDTH-1)
                {
                    neighbor_sum += partial_board[i+1][0];
                }
                else
                {
                    neighbor_sum += partial_board[i+1][j+1];

                }

                // WEST neighbor
                if (j == 0)
                {
                    neighbor_sum += partial_board[i][WIDTH-1];
                }
                else
                {
                    neighbor_sum += partial_board[i][j-1];
                }

                // EAST neighbor
                if (j == WIDTH-1)
                {
                    neighbor_sum += partial_board[i][0];
                }
                else
                {
                    neighbor_sum += partial_board[i][j+1];
                }
            }

            // update next_state
            if (neighbor_sum < 3 || neighbor_sum > 5)
            {
                next_state[i][j] = 0;
            }
            else 
            {
                next_state[i][j] = 1;
            }
        }
    }

    // Call MPI_Barrier to ensure all processes have finished updating their partial boards
    MPI_Barrier(MPI_COMM_WORLD);

    // copy next_state to partial_board
    for (int i = 0; i < HEIGHT/p; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            partial_board[i][j] = next_state[i][j];
        }
    }
}

void Simulate(int partial_board[][WIDTH], int rank, int p, int num_iterations)
{
    // run DetermineState() for num_iterations
    for (int i = 0; i < num_iterations; i++)
    {
        printf("Proc %d iteration %d\n", rank, i);

        DetermineState(partial_board, rank, p);

        // print every 10 iterations
        if (i % 10 == 0)
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
    printf("Proc %d: line 420\n", rank);
    GenerateInitialGOL(partial_board, rank, p);
    printf("Proc %d: line 422\n", rank);
    //PrintBoard(partial_board, rank, p);

    Simulate(partial_board, rank, p, NUM_ITERATIONS);

    MPI_Finalize();
    return 0;
}