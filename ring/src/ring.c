#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>

int main(int argc, char** argv) {

	bool message = true;
	int comm_sz;
	int my_rank;

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	if (my_rank == 0) {
		printf("I'm %d, sending %d to process %d!\n", my_rank, message, my_rank+1);
		
		MPI_Send(&message, 1, MPI_C_BOOL, 1, 0, MPI_COMM_WORLD);
	}
	else {
		MPI_Recv(&message, 1, MPI_C_BOOL, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("\tI'm %d, received %d from process %d!\n", my_rank, message, my_rank-1);

		if (my_rank != comm_sz-1) {
			MPI_Send(&message, 1, MPI_C_BOOL, my_rank+1, 0, MPI_COMM_WORLD);
			printf("I'm %d, sending %d to process %d!\n", my_rank, message, my_rank+1);
		}
	}
	
	MPI_Finalize();
}