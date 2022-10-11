#include <stdio.h>
#include <math.h>
#include <mpi.h>


const int first = 0b0001; // 0b0001
const int second  = 0b0010; // 0b0010
const int third = 0b0100; // 0b0100

int main(int argc, char** argv)
{
	int rank, p;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int x[1024];
	for (int i = 0; i < 1024; i++)
	{
		x[i] = rank+1;
	}

	int sum = 0;
	for (int i = 0; i < 1024; i++)
	{
		sum += x[i];
	}

	double start = MPI_Wtime();

	int time_steps = 0;

	int j = 1;
	while (j < p)
	{
		j *= 2;
		time_steps++;
	}

	for (int i = 0; i < time_steps; i++)
	{
		MPI_Send(&sum, 1, MPI_INT, rank^(1<<i), 0, MPI_COMM_WORLD);
		int recv_sum = 0;
		MPI_Recv(&recv_sum, 1, MPI_INT, rank^(1<<i), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		sum += recv_sum;
	}	

	double end = MPI_Wtime();

	printf("My all reduce implementation time taken: %lf microseconds.\n", (end-start)*1000000);

	MPI_Finalize();
	return 0;
}
