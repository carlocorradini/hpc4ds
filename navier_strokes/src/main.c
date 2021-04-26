#include <stdlib.h>
#include "mpi.h"

#define WORLD_WIDTH 10
#define WORLD_WIDTH_BOUNDS (WORLD_WIDTH + 2)
#define WORLD_HEIGHT 10
#define WORLD_HEIGHT_BOUNDS (WORLD_HEIGHT + 2)
#define VISCOSITY 0.0001
#define DIFFUSION 0.0001
#define DENSITY 10
#define TIME_STEP 0.01

static double **u = NULL;
static double **u_prev = NULL;
static double **v = NULL;
static double **v_prev = NULL;
static double **dense = NULL;
static double **dense_prev = NULL;

static void tick(void);

static void increase_density(size_t x, size_t y);

static void applyForce(size_t cellX, size_t cellY, double vX, double vY);

static void swap(double **x0, double **x);

static void add_source(double **x, double **s, double time_step);

static void diffuse(size_t b, double **x, double **x0, double diff, double time_step);

static void advect(size_t b, double **d, double **d0, double **uu, double **vv, double time_step);

static void set_bnd(size_t b, double **x);

static void dens_step(double **x, double **x0, double **uu, double **vv, double diff, double time_step);

static void vel_step(double **uu, double **vv, double **u0, double **v0, double viscosity, double time_step);

static void project(double **uu, double **vv, double **p, double **div);

int main(void) {
    int rank, size;

    // TODO RICORDARSI DI AGGIUNGERE BOUNDARIES +2 IN X E +2 IN Y

    // Allocate u & u_prev
    u = (double **) calloc(WORLD_HEIGHT_BOUNDS, sizeof(double *));
    for (size_t i = 0; i < WORLD_WIDTH_BOUNDS; ++i) u[i] = (double *) calloc(WORLD_WIDTH_BOUNDS, sizeof(double *));
    u_prev = (double **) calloc(WORLD_HEIGHT_BOUNDS, sizeof(double *));
    for (size_t i = 0; i < WORLD_WIDTH_BOUNDS; ++i) u_prev[i] = (double *) calloc(WORLD_WIDTH_BOUNDS, sizeof(double *));


    // Allocate v & v_prev
    v = (double **) calloc(WORLD_HEIGHT_BOUNDS, sizeof(double *));
    for (size_t i = 0; i < WORLD_WIDTH_BOUNDS; ++i) v[i] = (double *) calloc(WORLD_WIDTH_BOUNDS, sizeof(double *));
    v_prev = (double **) calloc(WORLD_HEIGHT_BOUNDS, sizeof(double *));
    for (size_t i = 0; i < WORLD_WIDTH_BOUNDS; ++i) v_prev[i] = (double *) calloc(WORLD_WIDTH_BOUNDS, sizeof(double *));

    // Allocate dense & dense_prev
    dense = (double **) calloc(WORLD_HEIGHT_BOUNDS, sizeof(double *));
    for (size_t i = 0; i < WORLD_WIDTH_BOUNDS; ++i) dense[i] = (double *) calloc(WORLD_WIDTH_BOUNDS, sizeof(double *));
    dense_prev = (double **) calloc(WORLD_HEIGHT_BOUNDS, sizeof(double *));
    for (size_t i = 0; i < WORLD_WIDTH_BOUNDS; ++i)
        dense_prev[i] = (double *) calloc(WORLD_WIDTH_BOUNDS, sizeof(double *));

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //
    tick();
    increase_density(WORLD_WIDTH / 2, WORLD_HEIGHT / 2);
    applyForce(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, 1.5, 1.7);
    increase_density(WORLD_WIDTH / 2, WORLD_HEIGHT / 2);
    tick();
    increase_density(WORLD_WIDTH / 2, WORLD_HEIGHT / 2);
    tick();
    //

    MPI_Finalize();
    return EXIT_SUCCESS;
}

static void tick(void) {
    vel_step(u, v, u_prev, v_prev, VISCOSITY, TIME_STEP);
    dens_step(dense, dense_prev, u, v, DIFFUSION, TIME_STEP);
}

static void increase_density(size_t x, size_t y) {
    // TODO add bound checking
    dense[x][y] += DENSITY;
}

//////////////////////////////////////////////////////////

static void applyForce(size_t cellX, size_t cellY, double vX, double vY) {
    const double dX = u[cellX][cellY];
    const double dY = v[cellX][cellY];

    // TODO scrivere meglio
    u[cellX][cellY] = vX != 0 ? vX : dX;
    v[cellX][cellY] = vY != 0 ? vY : dY;
}

/**
 *  TODO It may be better this way
 *  Remember to call swap(&x, &y) with the reference, 
 *  not just the variable name
 *  not just the variable nameT
 */
static void swap(double **x0, double **x) {
    double *tmp = *x0;
    *x0 = *x;
    *x = tmp;
}

static void add_source(double **_x, double **s, double time_step) {
    for (size_t y = 0; y < WORLD_HEIGHT_BOUNDS; ++y) {
        for (size_t x = 0; x < WORLD_WIDTH_BOUNDS; ++x) {
            _x[y][x] += time_step * s[y][x];
        }
    }
}

static void diffuse(size_t b, double **_x, double **x0, double diff, double time_step) {
    double a = time_step * diff * WORLD_WIDTH * WORLD_HEIGHT;

    //TODO 20? MAYBE ITERATIONS? thread???
    for (size_t k = 0; k < 20; k++) {
        for (size_t y = 1; y <= WORLD_HEIGHT; ++y) {
            for (size_t x = 1; x <= WORLD_WIDTH; ++x) {
                _x[y][x] = (x0[y][x] + a * (_x[y][x - 1] + _x[y][x + 1] + _x[y - 1][x] + _x[y + 1][x]))
                           / (1 + 4 * a);
            }
        }

        set_bnd(b, _x);
    }
}

static void advect(size_t b, double **d, double **d0, double **uu, double **vv, double time_step) {
    // TODO controlla source dato che utilizza N
    double dt0_width = time_step * (double) WORLD_WIDTH;
    double dt0_height = time_step * (double) WORLD_HEIGHT;

    // TODO Cambia i nomi please
    for (size_t _y = 1; _y <= WORLD_HEIGHT; ++_y) {
        for (size_t _x = 1; _x <= WORLD_WIDTH; ++_x) {
            double x = (double) _x - dt0_width * uu[_y][_x];
            double y = (double) _y - dt0_height * vv[_y][_x];

            // Check x
            if (x < 0.5)
                x = 0.5;
            if (x > WORLD_WIDTH + 0.5)
                x = WORLD_WIDTH + 0.5;
            size_t x0 = (size_t) x;
            size_t x1 = x0 + 1;

            // Check y
            if (y < 0.5)
                y = 0.5;
            if (y > WORLD_HEIGHT + 0.5)
                y = WORLD_HEIGHT + 0.5;
            size_t y0 = (size_t) y;
            size_t y1 = y0 + 1;


            double s1 = x - (double) x0;
            double s0 = 1 - s1;
            double t1 = y - (double) y0;
            double t0 = 1 - t1;

            d[_y][_x] = s0 * (t0 * d0[y0][x0] + t1 * d0[y1][x0])
                        + s1 * (t0 * d0[y0][x1] + t1 * d0[y1][x1]);
        }
    }

    set_bnd(b, d);
}

static void set_bnd(size_t b, double **_x) {
    for (size_t y = 1; y <= WORLD_HEIGHT; ++y) {
        for (size_t x = 1; x <= WORLD_WIDTH; ++x) {
            _x[y][0] = (b == 1) ? -_x[y][1] : _x[y][1];
            _x[y][WORLD_WIDTH + 1] = b == 1 ? -_x[y][WORLD_WIDTH] : _x[y][WORLD_WIDTH];
            _x[0][x] = b == 2 ? -_x[1][x] : _x[1][x];
            _x[WORLD_HEIGHT + 1][x] = b == 2 ? -_x[WORLD_HEIGHT][x] : _x[WORLD_HEIGHT][x];
        }
    }

    _x[0][0] = 0.5 * (_x[0][1] + _x[1][0]);
    _x[WORLD_HEIGHT + 1][0] = 0.5 * (_x[WORLD_HEIGHT + 1][1] + _x[WORLD_HEIGHT][0]);
    _x[0][WORLD_WIDTH + 1] = 0.5 * (_x[0][WORLD_WIDTH] + _x[1][WORLD_WIDTH + 1]);
    _x[WORLD_HEIGHT + 1][WORLD_WIDTH + 1] =
            0.5 * (_x[WORLD_HEIGHT + 1][WORLD_WIDTH] + _x[WORLD_HEIGHT][WORLD_WIDTH + 1]);
}

static void dens_step(double **x, double **x0, double **uu, double **vv, double diff, double time_step) {
    // TODO does swapping twice make sense?
    swap(x0, x);
    diffuse(0, x, x0, diff, time_step);
    swap(x0, x);
    advect(0, x, x0, uu, vv, time_step);
}

static void vel_step(double **uu, double **vv, double **u0, double **v0, double viscosity, double time_step) {
    add_source(uu, u0, time_step);
    add_source(vv, v0, time_step);
    swap(u0, uu);
    diffuse(1, uu, u0, viscosity, time_step);
    swap(v0, vv);
    diffuse(2, vv, v0, viscosity, time_step);
    project(uu, vv, u0, v0);
    swap(u0, uu);
    swap(v0, vv);
    advect(1, uu, u0, u0, v0, time_step);
    advect(2, vv, v0, u0, v0, time_step);
    project(uu, vv, u0, v0);
}

static void project(double **uu, double **vv, double **p, double **div) {
    // TODO ??? N CONTROLLA SOURCE
    double h = 1.0 / WORLD_WIDTH;

    for (size_t y = 1; y <= WORLD_HEIGHT; ++y) {
        for (size_t x = 1; x <= WORLD_WIDTH; ++x) {
            div[y][x] = -0.5 * h
                        * (uu[y][x + 1] - uu[y][x - 1] + vv[y + 1][x] - vv[y - 1][x]);
            p[y][x] = 0;
        }
    }

    set_bnd(0, div);
    set_bnd(0, p);

    // TODO k = 20 wtf iterations?
    for (size_t k = 0; k < 20; k++) {
        for (size_t y = 1; y <= WORLD_HEIGHT; ++y) {
            for (size_t x = 1; x <= WORLD_WIDTH; ++x) {
                p[y][x] = (div[y][x] + p[y][x - 1] + p[y][x + 1] + p[y - 1][x] + p[y + 1][x]) / 4;
            }
        }

        set_bnd(0, p);
    }

    for (size_t y = 1; y <= WORLD_HEIGHT; ++y) {
        for (size_t x = 1; x <= WORLD_WIDTH; ++x) {
            uu[y][x] -= 0.5 * (p[y][x + 1] - p[y][x - 1]) / h;
            vv[y][x] -= 0.5 * (p[y + 1][x] - p[y - 1][x]) / h;
        }
    }

    set_bnd(1, uu);
    set_bnd(2, vv);
}
