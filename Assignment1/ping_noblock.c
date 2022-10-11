#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_ITER 20
#define MB 1048576

#define SENDER_RANK 1
#define RECEIVER_RANK 0

int main(int argc, char** argv)
{
	// init mpi
	int rank, p;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	for (int i = 0; i <= MAX_ITER; i++)
	{
		double avg_send_time = 0, avg_recv_time = 0;
                int size = 1;
                for (int j = 0; j < i; j++)
                {
                	size *= 2;
                }
		
		/****Blocking Calls****/
		/*
		for (int k = 0; k < 10; k++)
		{
			if (rank == SENDER_RANK)
			{
				double start, end;
				
				char *send_buf = (char*)malloc(sizeof(char) * size);
				memset(send_buf, 0, sizeof(char) * size);
				for (int j = 0; j < size; j++) {send_buf[j] = 'E';}	

				start = MPI_Wtime();

				MPI_Send(send_buf, size, MPI_BYTE, RECEIVER_RANK, 0, MPI_COMM_WORLD);

				end = MPI_Wtime();

				double time_taken = (end - start) * 1000000;
				avg_send_time += time_taken;
				free(send_buf);
			}
			else if (rank == RECEIVER_RANK)
			{
				double start, end;

				char *recv_buf = (char*)malloc(sizeof(char) * size);
				memset(recv_buf, 0, sizeof(char) * size);

				start = MPI_Wtime();

				MPI_Recv(recv_buf, size, MPI_BYTE, SENDER_RANK, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	
				end = MPI_Wtime();
	
				double time_taken = (end - start) * 1000000;
				avg_recv_time += time_taken;
				free(recv_buf);
			}
		}
		*/

		/****Non-blocking Calls****/
		for (int k = 0; k < 10; k++)
		{
			if (rank == SENDER_RANK)
			{
				double start, end;

				char *send_buf = (char*)malloc(sizeof(char) * size);
				memset(send_buf, 0, sizeof(char) * size);
				for (int j = 0; j < size; j++) {send_buf[j] = 'E';};

				start = MPI_Wtime();

				MPI_Send(send_buf, size, MPI_BYTE, RECEIVER_RANK, 0, MPI_COMM_WORLD);

				end = MPI_Wtime();

				double time_taken = (end - start) * 1000000;
				avg_send_time += time_taken;
				free(send_buf);
			}
			else if (rank == RECEIVER_RANK)
			{
				double start, end;
				MPI_Request rq;
				char *recv_buf = (char*)malloc(sizeof(char) * size);

				start = MPI_Wtime();

				MPI_Irecv(recv_buf, size, MPI_BYTE, SENDER_RANK, 0, MPI_COMM_WORLD, &rq);

				
				end = MPI_Wtime();
				double time_taken = (end - start) * 1000000;
				avg_recv_time += time_taken;
				
				MPI_Wait(&rq, MPI_STATUS_IGNORE);
				free(recv_buf);	
			}
		}

		if (rank == SENDER_RANK)
		{
			printf("rank=%d: sent %d bytes taking avg of %f microseconds\n", rank, size, avg_send_time / 10);
		}
		else if (rank == RECEIVER_RANK)
		{
			printf("rank=%d: received %d bytes taking avg of %f microseconds\n", rank, size, avg_recv_time / 10);
		}
	}	

	MPI_Finalize();
}
