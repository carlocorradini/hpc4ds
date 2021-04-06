#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

#define EXPONENT 20
#define PRECISION 16

// Byte type alias
typedef uint8_t byte_t;

// Ping Pong data
typedef struct pp_data {
    // Bytes sent
    uint32_t bytes;
    // Transfer time in microseconds
    uint64_t transfer_time_us;
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
        pp_data data[EXPONENT + 1];

        for (int i = 0; i <= EXPONENT; i++) {
            struct timeval start, end;
            uint64_t transfer_time = 0;
            uint32_t bytes = (uint32_t) pow(2, i);
            // Allocate a buffer just big enough to hold the payload
            byte_t *buffer = (byte_t *) calloc(bytes, sizeof(byte_t));
            if (buffer == NULL) {
                fprintf(stderr, "Unable to allocate a buffer of %d bytes", bytes);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }

            for (int j = 0; j < PRECISION; j++) {
                gettimeofday(&start, NULL);
                MPI_Send(buffer, bytes, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer, bytes, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                gettimeofday(&end, NULL);

                // Calculate time difference in us
                transfer_time += ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
            }
            transfer_time = transfer_time / (2 * PRECISION);

            data[i] = (pp_data) {bytes, transfer_time};

            free(buffer);
        }

        // Print data
        for (int i = 0; i <= EXPONENT; i++) {
            const pp_data d = data[i];
            const double transfer_time_s = d.transfer_time_us / (double) pow(10, 6);
            const double throughput = d.bytes / (transfer_time_s <= 1.0 ? 1.0 : transfer_time_s);
            printf("%8d bytes in %6ldus (~%lds) | Throughput ~%ld byte/s\n",
                   d.bytes, d.transfer_time_us, (uint64_t) round(transfer_time_s), (uint64_t) round(throughput));
        }
    } else if (rank == 1) {
        for (int i = 0; i <= EXPONENT; i++) {
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