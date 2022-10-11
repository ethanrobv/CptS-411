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


	int global_sum = 0;
	
	double start = MPI_Wtime();

	MPI_Allreduce(&sum, &global_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	double end = MPI_Wtime();

	printf("MPI all reduce implementation time: %lf microseconds.\n", (end-start)*1000000);

	MPI_Finalize();
	return 0;
}
