#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define A 0;
#define B 1;

double f(double x);

int main(void) {
    double a, b, h;
    int rank, size;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    a = A
    b = B
    h = (b - a) / (double) size;

    if (rank != 0) {
        double approx = f(a + rank * h);
        MPI_Send(&approx, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        double approx = (f(a) + f(b)) / 2.0L;
        printf("[%3d]: Partial area from %.8lf to %.8lf is %.8lf\n", 0, a, a + 1 * h, approx);

        for (int q = 1; q < size; ++q) {
            double approx_tmp;
            MPI_Status status;

            MPI_Recv(&approx_tmp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            printf("[%3d]: Partial area from %.8lf to %.8lf is %.8lf\n", status.MPI_SOURCE,
                   a + status.MPI_SOURCE * h, a + ((status.MPI_SOURCE + 1) * h), approx_tmp);

            approx += approx_tmp;
        }

        approx *= h;
        printf("----------------------------------------------------------------\n");
        printf("Total area of trapezoid from %.8lf to %.8lf is %.8lf\n", a, b, approx);
        printf("----------------------------------------------------------------\n");
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}

double f(double x) {
    return 1 / (1 + x * x);
}