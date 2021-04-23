#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

int main(void) {
    int rank, size;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Hi from %d of %d\n", rank, size);

    MPI_Finalize();
    return EXIT_SUCCESS;
}