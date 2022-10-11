//#define __DEBUG__

#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv)
{
	int rank,p;

	// MPI initializations
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	printf("rank=%d\n", rank);
	printf("rank=%d; num_procs=%d\n", rank, p);

#ifdef __DEBUG__
	while(1){}
#endif
	
	MPI_Finalize();
}
