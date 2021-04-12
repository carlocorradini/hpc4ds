#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>

int main(void) {
    int rank, size;
    bool message = true;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("I'm %d, sending %d to process %d!\n", rank, message, rank + 1);

        MPI_Send(&message, 1, MPI_C_BOOL, 1, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&message, 1, MPI_C_BOOL, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("I'm %d, received %d from process %d!\n", rank, message, rank - 1);

        if (rank != size - 1) {
            MPI_Send(&message, 1, MPI_C_BOOL, rank + 1, 0, MPI_COMM_WORLD);
            printf("I'm %d, sending %d to process %d!\n", rank, message, rank + 1);
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}