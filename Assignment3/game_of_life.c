/*
* Ethan Vincent
* Parallel implementation of Conway's Game of Life
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mpi.h>

#define __DEBUG__ 0

// Globals for time keeping
double total_runtime = 0.0;
double single_generation_runtime = 0.0;
double total_comm_time = 0.0;

// Globals for game board
int HEIGHT = 0;
int WIDTH = 0;

void GenerateInitialGOL(int partial_board[][WIDTH], int rank, int p)
{   
    // give each process a random seed (except p0, which uses system time)
    // record the communication time
    double comm_start = MPI_Wtime();
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
    // stop recording comm time
    double comm_end = MPI_Wtime();
    total_comm_time += comm_end - comm_start;

    // generate random values to determine cell state
    for (int i = 0; i < (HEIGHT/p); i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            partial_board[i][j] = rand() % 2;
        }
    }

    printf("Process %d has generated its initial board\n", rank);
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
        printf("\n");
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
    int my_top_row[WIDTH];
    int my_bottom_row[WIDTH];
    
    for (int i = 0; i < num_iterations; i++)
    {
        //printf("rank: %d, iteration: %d\n", rank, i);
        // MPI_Barrier to synchronize all procs
        MPI_Barrier(MPI_COMM_WORLD);

        if (__DEBUG__)
        {
            printf("Process %d has started iteration %d\n", rank, i);
        }

        // start recording time for single_generation metric
        double start_single_gen_time = MPI_Wtime();

        // copy top and bottom rows
        for (int j = 0; j < WIDTH; j++)
        {
            my_top_row[j] = partial_board[0][j];
            my_bottom_row[j] = partial_board[(HEIGHT/p)-1][j];
        }

        if (__DEBUG__)
        {
            printf("Process %d has copied top and bottom rows\n", rank);
        }

        /*
        * MPI Sends and Receives
        */
        // record communication time
        double comm_start = MPI_Wtime();
        if (rank == 0)
        {
            // use non blocking communication
            MPI_Request request1, request2;
            MPI_Status status1, status2;
            MPI_Irecv(bottom_row, WIDTH, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &request1);
            MPI_Irecv(top_row, WIDTH, MPI_INT, p-1, 2, MPI_COMM_WORLD, &request2);

            // p0 top row is p-1 bottom row
            MPI_Send(my_top_row, WIDTH, MPI_INT, p-1, 1, MPI_COMM_WORLD);

            MPI_Send(my_bottom_row, WIDTH, MPI_INT, rank+1, 2, MPI_COMM_WORLD);

            MPI_Wait(&request1, &status1);
            MPI_Wait(&request2, &status2);

            /*
            if (__DEBUG__)
            {
                printf("Process %d has sent top and bottom rows\n", rank);
            }

            // p-1 bottom row is p0 top row
            MPI_Recv(top_row, WIDTH, MPI_INT, p-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Recv(bottom_row, WIDTH, MPI_INT, rank+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (__DEBUG__)
            {
                printf("Process %d has received top and bottom rows\n", rank);
            }
            */
        }
        else if (rank == p-1)
        {
            // use non blocking communication
            MPI_Request request1, request2;
            MPI_Status status1, status2;
            MPI_Irecv(bottom_row, WIDTH, MPI_INT, 0, 1, MPI_COMM_WORLD, &request1);
            MPI_Irecv(top_row, WIDTH, MPI_INT, rank-1, 2, MPI_COMM_WORLD, &request2);

            // 0 top row is p-1 bottom row
            MPI_Send(my_bottom_row, WIDTH, MPI_INT, 0, 2, MPI_COMM_WORLD);

            MPI_Send(my_top_row, WIDTH, MPI_INT, rank-1, 1, MPI_COMM_WORLD);

            MPI_Wait(&request1, &status1);
            MPI_Wait(&request2, &status2);

            /*
            if (__DEBUG__)
            {
                printf("Process %d has sent top and bottom rows\n", rank);
            }

            // 0 top row is p-1 bottom row
            MPI_Recv(bottom_row, WIDTH, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Recv(top_row, WIDTH, MPI_INT, rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (__DEBUG__)
            {
                printf("Process %d has received top and bottom rows\n", rank);
            }
            */
        }
        else
        {
            // use non blocking communication
            MPI_Request request1, request2;
            MPI_Status status1, status2;
            MPI_Irecv(bottom_row, WIDTH, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &request1);
            MPI_Irecv(top_row, WIDTH, MPI_INT, rank-1, 2, MPI_COMM_WORLD, &request2);

            // send top row to proc above
            MPI_Send(my_top_row, WIDTH, MPI_INT, rank-1, 1, MPI_COMM_WORLD);
            // send bottom row to proc below
            MPI_Send(my_bottom_row, WIDTH, MPI_INT, rank+1, 2, MPI_COMM_WORLD);

            MPI_Wait(&request1, &status1);
            MPI_Wait(&request2, &status2);

            /*
            if (__DEBUG__)
            {
                printf("Process %d has sent top and bottom rows\n", rank);
            }

            // receive bottom row from proc above
            MPI_Recv(top_row, WIDTH, MPI_INT, rank-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // receive top row from proc below
            MPI_Recv(bottom_row, WIDTH, MPI_INT, rank+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (__DEBUG__)
            {
                printf("Process %d has received top and bottom rows\n", rank);
            }
            */
        }


        // end recording communication time
        double comm_end = MPI_Wtime();
        total_comm_time += comm_end - comm_start;

        /*
        * Determine new state
        */

        int new_board[(HEIGHT/p)][WIDTH];
        for (int x = 0; x < HEIGHT/p; x++)
        {
            for (int y = 0; y < WIDTH; y++)
            {
                new_board[x][y] = DetermineState(x, y, partial_board, top_row, bottom_row, rank, p);
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

        // end recording time for single_generation metric
        double end_single_gen_time = MPI_Wtime();
        double single_gen_time = end_single_gen_time - start_single_gen_time;

        /*
        if (i % 2 == 0 && __DEBUG__)
        {
            if (rank == 0)
            {
                printf("Iteration %d\n", i);
            }
            PrintBoard(partial_board, rank, p);
        }
        */

       if (__DEBUG__)
       {
            printf("Process %d has finished iteration %d\n", rank, i);
       }
    }
}

int main(int argc, char** argv)
{
    int rank,p;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (__DEBUG__ && rank == 0)
    {
        for (int i = 1; i < argc; i++)
        {
            printf("atoi(argv[%d]): %d\n", i, atoi(argv[i]));
        }
    }

    if (argc != 3)
    {
        if (rank == 0)
        {
            printf("usage: ./run_GOL.sh <num_iterations> <board_size> <num_threads>\n");
            printf("argv= ");
            for (int i = 0; i < argc; i++)
            {
                printf("%s ", argv[i]);
            }
            printf("\n");
        }
        return -1;
    }

    int _num_iterations = atoi(argv[1]);
    if (_num_iterations == 0)
    {
        if (rank == 0)
        {
            printf("invalid number of iterations\n");
            printf("argv= ");
            for (int i = 0; i < argc; i++)
            {
                printf("%s ", argv[i]);
            }
            printf("\n");
        }
        
        return -1;
    }

    int _board_size = atoi(argv[2]);
    if (_board_size == 0)
    {
        if (rank == 0)
        {
            printf("invalid board size\n");
            printf("argv= ");
            for (int i = 0; i < argc; i++)
            {
                printf("%s ", argv[i]);
            }
            printf("\n");
        }
        
        return -1;
    }

    if (rank == 0)
    {
        _num_iterations = atoi(argv[1]);
        _board_size = atoi(argv[2]);
        if (_num_iterations == 0 || _board_size == 0)
        {
            printf("usage: ./run_GOL.sh <num_iterations> <board_size> <num_threads>\n");
            printf("argv= ");
            for (int i = 0; i < argc; i++)
            {
                printf("%s ", argv[i]);
            }
            printf("\n");
            return -1;
        }
        
        // send num_iterations and board_size to all processes
        for (int i = 1; i < p; i++)
        {
            MPI_Send(&_num_iterations, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&_board_size, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
    }
    else
    {
        // receive num_iterations and board_size from process 0
        MPI_Recv(&_num_iterations, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&_board_size, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    HEIGHT = WIDTH = _board_size;


    // start recording time for total_runtime metric
    double start_total_runtime = MPI_Wtime();

    int partial_board[(HEIGHT/p)][WIDTH];

    GenerateInitialGOL(partial_board, rank, p);

    Simulate(partial_board, rank, p, _num_iterations);

    // end recording time for total_runtime metric
    double end_total_runtime = MPI_Wtime();
    double total_runtime = end_total_runtime - start_total_runtime;

    
    /*
    * Output metrics
    */
    // get max of each metric
    MPI_Allreduce(MPI_IN_PLACE, &total_runtime, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(MPI_IN_PLACE, &total_comm_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(MPI_IN_PLACE, &single_generation_runtime, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("--------------------------\n");
        printf("num_procs = %d    num_iterations = %d    board_size = %d\n", p, _num_iterations, _board_size);
        printf("--------------------------\n");
        printf("total runtime=%lf microseconds\n", total_runtime*1000000);
        printf("average single generation time=%lf microseconds\n", (total_runtime/_num_iterations)*1000000);
        printf("communication time=%lf microseconds\n", total_comm_time*1000000);
        printf("total computation time=%lf microseconds\n", (total_runtime - total_comm_time)*1000000);
    }

    MPI_Finalize();
    return 0;
}