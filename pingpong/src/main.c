#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

#define BYTE_EXPONENT 20
#define PRECISION 10

// Byte type alias
typedef size_t byte_t;

int main(int argc, char **argv) {
    const int size, rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (size != 2) {
        fprintf(stderr, "Ping Pong must use two processes. Current size is %ld\n", size);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (rank == 0) {
        double bandwidths[BYTE_EXPONENT + 1];

        for (int i = 0; i <= BYTE_EXPONENT; i++) {
            time_t start, end, current, transfer_time;
            const size_t bytes = (size_t) pow(2, i);
            const byte_t *buffer = (byte_t *) malloc(bytes * sizeof(byte_t));
            current = 0;

            for (int j = 0; j < PRECISION; j++) {
                gettimeofday(&start, NULL);
                MPI_Send(buffer, bytes, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer, bytes, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                gettimeofday(&end, NULL);

                current += difftime(end, start);
            }

            transfer_time = current / (2.0 * PRECISION);

            bandwidths[i] = bytes / transfer_time;

            free(buffer);
        }

        for (int i = 0; i <= BYTE_EXPONENT; i++) {
            printf("Bandwidth for %zu bytes: %lf b/s\n", (size_t) pow(2, i), bandwidths[i]);
        }
    } else if (rank == 1) {
        for (int i = 0; i <= BYTE_EXPONENT; i++) {
            MPI_Status status;
            size_t bytes;

            MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_BYTE, &bytes);

            // Allocate a buffer just big enough to hold the incoming buffer
            uint8_t *buffer = (byte_t *) malloc(bytes * sizeof(byte_t));

            for (int j = 0; j < PRECISION; j++) {
                MPI_Recv(buffer, bytes, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(buffer, bytes, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
            }

            free(buffer);
        }
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}