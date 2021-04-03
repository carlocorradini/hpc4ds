#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

#define BYTE_EXPONENT 20
#define PRECISION 10

// Byte type alias
typedef uint8_t byte_t;

// Ping Pong data
typedef struct pp_data {
    uint32_t bytes;
    double time;
} pp_data;

int main(int argc, char **argv) {
    int size, rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Check if world size is exactly 2
    if (size != 2) {
        fprintf(stderr, "Ping Pong must use two processes. Current size is %d\n", size);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (rank == 0) {
        // Result data
        pp_data data[BYTE_EXPONENT + 1];

        for (int i = 0; i <= BYTE_EXPONENT; i++) {
            double start, end, transfer_time;
            uint32_t bytes = (uint32_t) pow(2, i);
            // Allocate a buffer just big enough to hold the payload
            byte_t *buffer = (byte_t *) calloc(bytes, sizeof(byte_t));
            if (buffer == NULL) {
                fprintf(stderr, "Unable to allocate a buffer of %d bytes", bytes);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }

            transfer_time = 0;
            for (int j = 0; j < PRECISION; j++) {
                start = MPI_Wtime();
                MPI_Send(buffer, bytes, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer, bytes, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                end = MPI_Wtime();

                transfer_time += end - start;
            }
            transfer_time = transfer_time / (2.0 * PRECISION);

            data[i] = (pp_data) {bytes, transfer_time};

            free(buffer);
        }

        for (int i = 0; i <= BYTE_EXPONENT; i++) {
            printf("Bandwidth for %d bytes: %f b/s\n", data[i].bytes, data[i].bytes / data[i].time);
        }
    } else if (rank == 1) {
        for (int i = 0; i <= BYTE_EXPONENT; i++) {
            MPI_Status status;
            uint32_t bytes;

            MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_BYTE, &bytes);

            // Allocate a buffer just big enough to hold the incoming buffer
            uint8_t *buffer = (byte_t *) calloc(bytes, sizeof(byte_t));
            if (buffer == NULL) {
                fprintf(stderr, "Unable to allocate a buffer of %d bytes", bytes);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }

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