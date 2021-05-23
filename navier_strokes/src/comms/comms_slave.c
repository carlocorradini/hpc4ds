#include "comms_slave.h"
#include <mpi.h>

void do_slave(void) {
    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

}
