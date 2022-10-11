#include <stdio.h>
#include <mpi.h>

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

	// send each proc's data to right neighbor,
	// proc p-1 adds it's recv data to its own partial sum,
	// other procs overwrite their local partial sum with their recveived one.
	// p-1 time steps
	for (int i = 0; i < p-1; i++)
	{
		if (rank < (p-1))
		{
			MPI_Send(&sum, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
		}
		if (rank > 0)
		{
			int recv_buf = 0;
			MPI_Recv(&recv_buf, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if (rank == p-1)
			{
				sum += recv_buf;
				printf("p%d: sum=%d\n", rank, sum);
			}
			else
			{
				sum = recv_buf;
			}
		}
	}

	for (int i = p; i > 0; i--)
	{
		if (rank > 0)
		{
			MPI_Send(&sum, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
		}
		if (rank < (p-1))
		{
			MPI_Recv(&sum, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}

	double end = MPI_Wtime();

	printf("Naive implementation time taken: %lf microseconds.\n", (end-start) * 1000000);

	MPI_Finalize();
	return 0;
}
