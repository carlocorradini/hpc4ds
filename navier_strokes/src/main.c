#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mpi.h"

#define N 10

static double N_INVERSE = 1 / (double) N;
static size_t SIZE = (N + 2) * (N + 2);
static double *u = NULL;
static double *v = NULL;
static double *dense = NULL;
static double *u_prev = NULL;
static double *v_prev = NULL;
static double *dense_prev = NULL;

static size_t index_calc(size_t i, size_t j);

static double getDx(size_t x, size_t y);

static double getDy(size_t x, size_t y);

static void applyForce(size_t cellX, size_t cellY, double vX, double vY);

static void tick(double dT, double viscosity, double diff);

static double *getInverseWarpPosition(double x, double y, double scale);

static double lerp(double x0, double x1, double l);

static void swap(double *x0, double *x);

static void add_source(double *x, const double *s, double dT);

static void diffuse(size_t b, double *x, const double *x0, double diff, double dT);

static void advect(size_t b, double *d, const double *d0, const double *uu, const double *vv, double dT);

static void set_bnd(size_t b, double *x);

static void dens_step(double *x, double *x0, double *uu, double *vv, double diff, double dT);

static void vel_step(double *uu, double *vv, double *u0, double *v0, double viscosity, double dT);

static void project(double *uu, double *vv, double *p, double *div);

int main(void) {
    int rank, size;
    u = (double *) calloc(SIZE, sizeof(double));
    v = (double *) calloc(SIZE, sizeof(double));
    dense = calloc(SIZE, sizeof(double));
    u_prev = calloc(SIZE, sizeof(double));
    v_prev = calloc(SIZE, sizeof(double));
    dense_prev = calloc(SIZE, sizeof(double));

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //
    tick(0.01, 0.0001, 0.0001);
    dense[index_calc(N / 2, N / 2)] += 10;
    applyForce(N / 2, N / 2, 1.5, 1.7);
    dense[index_calc(N / 2, N / 2)] += 5;
    tick(0.01, 0.0001, 0.0001);
    dense[index_calc(N / 2, N / 2)] += 1;
    tick(0.01, 0.0001, 0.0001);

    for (size_t x = 0; x < N + 2; ++x) {
        for (size_t y = 0; y < N + 2; ++y) {
            double density = dense[index_calc(x, y)];
            printf("[{d: %lf}]   ", density);
        }
        printf("\n");
    }

    //

    MPI_Finalize();
    return EXIT_SUCCESS;
}

static size_t index_calc(size_t i, size_t j) {
    return i + (N + 2) * j;
}

static double getDx(size_t x, size_t y) {
    return u[index_calc(x + 1, y + 1)];
}

static double getDy(size_t x, size_t y) {
    return v[index_calc(x + 1, y + 1)];
}

static void applyForce(size_t cellX, size_t cellY, double vX, double vY) {
    const double dX = u[index_calc(cellX, cellY)];
    const double dY = v[index_calc(cellX, cellY)];

    u[index_calc(cellX, cellY)] = vX != 0 ? vX : dX;
    v[index_calc(cellX, cellY)] = vY != 0 ? vY : dY;
}

static void tick(double dT, double viscosity, double diff) {
    vel_step(u, v, u_prev, v_prev, viscosity, dT);
    dens_step(dense, dense_prev, u, v, diff, dT);
}

static double *getInverseWarpPosition(double x, double y, double scale) {
    // TODO Struct here
    double *result = calloc(2, sizeof(double));

    size_t cellX = floor(x * (double) N);
    size_t cellY = floor(y * (double) N);

    double cellU = (x * (double) N - ((double) cellX)) * N_INVERSE;
    double cellV = (y * (double) N - ((double) cellY)) * N_INVERSE;

    cellX += 1;
    cellY += 2;

    result[0] = (cellU > 0.5)
                ? lerp(u[index_calc(cellX, cellY)], u[index_calc(cellX + 1, cellY)], cellU - 0.5)
                : lerp(u[index_calc(cellX - 1, cellY)], u[index_calc(cellX, cellY)], 0.5 - cellU);
    result[1] = (cellV > 0.5)
                ? lerp(v[index_calc(cellX, cellY)], v[index_calc(cellX, cellY + 1)], cellU)
                : lerp(v[index_calc(cellX, cellY)], v[index_calc(cellX, cellY - 1)], 0.5 - cellV);

    result[0] *= -scale;
    result[1] *= -scale;

    result[0] += x;
    result[1] += y;

    return result;
}

static double lerp(double x0, double x1, double l) {
    l *= 1; //TODO wtf?
    return (1 - l) * x0 + l * x1;
}

static void swap(double *x0, double *x) {
    // TODO Come on do better here
    for (size_t i = 0; i < SIZE; ++i) {
        double tmp = x0[i];
        x0[i] = x[i];
        x[i] = tmp;
    }
}

static void add_source(double *x, const double *s, double dT) {
    size_t size = (N + 2) * (N + 2);
    for (size_t i = 0; i < size; i++)
        x[i] += dT * s[i];
}

static void diffuse(size_t b, double *x, const double *x0, double diff, double dT) {
    size_t i, j, k;
    double a = dT * diff * (double) N * (double) N;

    // TODO 20???
    for (k = 0; k < 20; k++) {
        for (i = 1; i <= N; i++) {
            for (j = 1; j <= N; j++) {
                x[index_calc(i, j)] =
                        (x0[index_calc(i, j)]
                         + a * (x[index_calc(i - 1, j)]
                                + x[index_calc(i + 1, j)]
                                + x[index_calc(i, j - 1)]
                                + x[index_calc(i, j + 1)]))
                        / (1 + 4 * a);
            }
        }

        set_bnd(b, x);
    }
}

static void advect(size_t b, double *d, const double *d0, const double *uu, const double *vv, double dT) {
    size_t i, j, i0, j0, i1, j1;
    double x, y, s0, t0, s1, t1, dt0;

    dt0 = dT * (double) N;
    for (i = 1; i <= N; i++) {
        for (j = 1; j <= N; j++) {
            x = (double) i - dt0 * uu[index_calc(i, j)];
            y = (double) j - dt0 * vv[index_calc(i, j)];

            if (x < 0.5) x = 0.5;
            if (x > (double) N + 0.5) x = (double) N + 0.5;

            i0 = (size_t) x;
            i1 = i0 + 1;

            if (y < 0.5) y = 0.5;
            if (y > (double) N + 0.5) y = (double) N + 0.5;

            j0 = (size_t) y;
            j1 = j0 + 1;
            s1 = x - (double) i0;
            s0 = 1 - s1;
            t1 = y - (double) j0;
            t0 = 1 - t1;
            d[index_calc(i, j)] = s0 * (t0 * d0[index_calc(i0, j0)] + t1 * d0[index_calc(i0, j1)])
                                  + s1 * (t0 * d0[index_calc(i1, j0)] + t1 * d0[index_calc(i1, j1)]);
        }
    }

    set_bnd(b, d);
}

static void set_bnd(size_t b, double *x) {
    for (size_t i = 1; i <= N; i++) {
        x[index_calc(0, i)] = (b == 1) ? -x[index_calc(1, i)] : x[index_calc(1, i)];
        x[index_calc(N + 1, i)] = b == 1 ? -x[index_calc(N, i)] : x[index_calc(N, i)];
        x[index_calc(i, 0)] = b == 2 ? -x[index_calc(i, 1)] : x[index_calc(i, 1)];
        x[index_calc(i, N + 1)] = b == 2 ? -x[index_calc(i, N)] : x[index_calc(i, N)];
    }

    x[index_calc(0, 0)] = 0.5 * (x[index_calc(1, 0)] + x[index_calc(0, 1)]);
    x[index_calc(0, N + 1)] = 0.5 * (x[index_calc(1, N + 1)] + x[index_calc(0, N)]);
    x[index_calc(N + 1, 0)] = 0.5 * (x[index_calc(N, 0)] + x[index_calc(N + 1, 1)]);
    x[index_calc(N + 1, N + 1)] = 0.5 * (x[index_calc(N, N + 1)] + x[index_calc(N + 1, N)]);
}

static void dens_step(double *x, double *x0, double *uu, double *vv, double diff, double dT) {
    //add_source(x, x0, dt);
    swap(x0, x);
    diffuse(0, x, x0, diff, dT);
    swap(x0, x);
    advect(0, x, x0, uu, vv, dT);
}

static void vel_step(double *uu, double *vv, double *u0, double *v0, double viscosity, double dT) {
    add_source(uu, u0, dT);
    add_source(vv, v0, dT);
    swap(u0, uu);
    diffuse(1, uu, u0, viscosity, dT);
    swap(v0, vv);
    diffuse(2, vv, v0, viscosity, dT);
    project(uu, vv, u0, v0);
    swap(u0, uu);
    swap(v0, vv);
    advect(1, uu, u0, u0, v0, dT);
    advect(2, vv, v0, u0, v0, dT);
    project(uu, vv, u0, v0);
}

static void project(double *uu, double *vv, double *p, double *div) {
    size_t i, j, k;
    double h = 1 / (double) N;

    for (i = 1; i <= N; i++) {
        for (j = 1; j <= N; j++) {
            div[index_calc(i, j)] =
                    -0.5 * h * (uu[index_calc(i + 1, j)] - uu[index_calc(i - 1, j)] + vv[index_calc(i, j + 1)] - vv[index_calc(i,
                                                                                                                j - 1)]);
            p[index_calc(i, j)] = 0;
        }
    }

    set_bnd(0, div);
    set_bnd(0, p);

    // TODO 20?
    for (k = 0; k < 20; k++) {
        for (i = 1; i <= N; i++) {
            for (j = 1; j <= N; j++) {
                p[index_calc(i, j)] = (div[index_calc(i, j)]
                                       + p[index_calc(i - 1, j)]
                                       + p[index_calc(i + 1, j)]
                                       + p[index_calc(i, j - 1)]
                                       + p[index_calc(i, j + 1)])
                                      / 4;
            }
        }
        set_bnd(0, p);
    }

    for (i = 1; i <= N; i++) {
        for (j = 1; j <= N; j++) {
            uu[index_calc(i, j)] -= 0.5 * (p[index_calc(i + 1, j)] - p[index_calc(i - 1, j)]) / h;
            vv[index_calc(i, j)] -= 0.5 * (p[index_calc(i, j + 1)] - p[index_calc(i, j - 1)]) / h;
        }
    }

    set_bnd(1, uu);
    set_bnd(2, vv);
}