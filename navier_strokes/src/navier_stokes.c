#include "navier_stokes.h"
#include <stdlib.h>
#include "utils.h"

typedef struct navier_stokes_t {
    // World
    size_t world_width;
    size_t world_width_bounds;
    size_t world_height;
    size_t world_height_bounds;
    double **u;
    double **u_prev;
    double **v;
    double **v_prev;
    double **dense;
    double **dense_prev;
    // Fluid
    double viscosity;
    double density;
    double diffusion;
    // Time
    double time_step;

} navier_stokes_t;

// PRIVATE DEFINITIONS
static void velocity_step(navier_stokes_t *ns);

static void density_step(navier_stokes_t *ns);

static void add_source_to_target(const navier_stokes_t *ns, double **target, const double **source);

static void
diffuse(const navier_stokes_t *ns, size_t bounds, double diffusion_value, double **target, const double **source);

static void project(navier_stokes_t *ns);

static void advect(const navier_stokes_t *ns, size_t bounds, double **d, double **d0, double **u, double **v);

static void set_bnd(const navier_stokes_t *ns, size_t bounds, double **target);

// PUBLIC
navier_stokes_t *ns_create(size_t world_width, size_t world_height,
                           double viscosity, double density, double diffusion,
                           double time_step) {
    navier_stokes_t *ns = (navier_stokes_t *) malloc(sizeof(navier_stokes_t));

    // World
    ns->world_width = world_width;
    ns->world_width_bounds = ns->world_width + 2;
    ns->world_height = world_height;
    ns->world_height_bounds = ns->world_height + 2;
    // Fluid
    ns->viscosity = viscosity;
    ns->density = density;
    ns->diffusion = diffusion;
    // Time
    ns->time_step = time_step;

    // Allocate u
    ns->u = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    for (size_t i = 0; i < ns->world_width_bounds; ++i)
        ns->u[i] = (double *) calloc(ns->world_width_bounds, sizeof(double *));

    // Allocate u_prev
    ns->u_prev = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    for (size_t i = 0; i < ns->world_width_bounds; ++i)
        ns->u_prev[i] = (double *) calloc(ns->world_width_bounds, sizeof(double *));

    // Allocate v
    ns->v = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    for (size_t i = 0; i < ns->world_width_bounds; ++i)
        ns->v[i] = (double *) calloc(ns->world_width_bounds, sizeof(double *));

    // Allocate v_prev
    ns->v_prev = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    for (size_t i = 0; i < ns->world_width_bounds; ++i)
        ns->v_prev[i] = (double *) calloc(ns->world_width_bounds, sizeof(double *));

    // Allocate dense
    ns->dense = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    for (size_t i = 0; i < ns->world_width_bounds; ++i)
        ns->dense[i] = (double *) calloc(ns->world_width_bounds, sizeof(double *));

    // Allocate dense_prev
    ns->dense_prev = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    for (size_t i = 0; i < ns->world_width_bounds; ++i)
        ns->dense_prev[i] = (double *) calloc(ns->world_width_bounds, sizeof(double *));

    return ns;
}

void tick(navier_stokes_t *ns) {
    velocity_step(ns);
    density_step(ns);
}

void increase_density(navier_stokes_t *ns, size_t x, size_t y) {
    // TODO check bounds
    ns->dense[y][x] += ns->density;
}

void apply_force(navier_stokes_t *ns, size_t cellX, size_t cellY, double vX, double vY) {
    // TODO check bounds
    const double dX = ns->u[cellX][cellY];
    const double dY = ns->v[cellX][cellY];

    // TODO scrivere meglio
    ns->u[cellX][cellY] = vX != 0 ? vX : dX;
    ns->v[cellX][cellY] = vY != 0 ? vY : dY;
}

const double **get_world(const navier_stokes_t *ns) {
    return (const double **) ns->dense;
}

// PRIVATE
static void velocity_step(navier_stokes_t *ns) {
    add_source_to_target(ns, ns->u, (const double **) ns->u_prev);
    add_source_to_target(ns, ns->v, (const double **) ns->v_prev);

    swap_array(ns->u_prev, ns->u, ns->world_width_bounds, ns->world_height_bounds);

    diffuse(ns, 1, ns->viscosity, ns->u, (const double **) ns->u_prev);
    swap_array(ns->v_prev, ns->v, ns->world_width_bounds, ns->world_height_bounds);
    diffuse(ns, 2, ns->viscosity, ns->v, (const double **) ns->v_prev);
    project(ns);
    swap_array(ns->u_prev, ns->u, ns->world_width_bounds, ns->world_height_bounds);
    swap_array(ns->v_prev, ns->v, ns->world_width_bounds, ns->world_height_bounds);
    advect(ns, 1, ns->u, ns->u_prev, ns->u_prev, ns->v_prev);
    advect(ns, 2, ns->v, ns->v_prev, ns->u_prev, ns->v_prev);
    project(ns);
}

static void density_step(navier_stokes_t *ns) {
    // TODO does swapping twice make sense?
    swap_array(ns->dense_prev, ns->dense, ns->world_width_bounds, ns->world_height_bounds);
    diffuse(ns, 0, ns->diffusion, ns->dense, (const double **) ns->dense_prev);
    swap_array(ns->dense_prev, ns->dense, ns->world_width_bounds, ns->world_height_bounds);
    advect(ns, 0, ns->dense, ns->dense_prev, ns->u, ns->v);
}

static void add_source_to_target(const navier_stokes_t *ns, double **target, const double **source) {
    for (size_t y = 0; y < ns->world_height_bounds; ++y) {
        for (size_t x = 0; x < ns->world_width_bounds; ++x) {
            target[y][x] += ns->time_step * source[y][x];
        }
    }
}

static void
diffuse(const navier_stokes_t *ns, size_t bounds, double diffusion_value, double **target, const double **source) {
    const double a = ns->time_step * diffusion_value * (double) ns->world_width * (double) ns->world_height;

    //TODO 20? MAYBE ITERATIONS? thread???
    for (size_t k = 0; k < 20; k++) {
        for (size_t y = 1; y <= ns->world_height; ++y) {
            for (size_t x = 1; x <= ns->world_width; ++x) {
                target[y][x] =
                        (source[y][x] + a * (target[y][x - 1] + target[y][x + 1] + target[y - 1][x] + target[y + 1][x]))
                        / (1 + 4 * a);
            }
        }

        set_bnd(ns, bounds, target);
    }
}

static void project(navier_stokes_t *ns) {
    // TODO ??? N CONTROLLA SOURCE
    double h = 1.0 / (double) ns->world_width;

    for (size_t y = 1; y <= ns->world_height; ++y) {
        for (size_t x = 1; x <= ns->world_width; ++x) {
            ns->v_prev[y][x] = -0.5 * h
                               * (ns->u[y][x + 1] - ns->u[y][x - 1] + ns->v[y + 1][x] - ns->v[y - 1][x]);
            ns->u_prev[y][x] = 0;
        }
    }

    set_bnd(ns, 0, ns->v_prev);
    set_bnd(ns, 0, ns->u_prev);

    // TODO k = 20 wtf iterations?
    for (size_t k = 0; k < 20; k++) {
        for (size_t y = 1; y <= ns->world_height; ++y) {
            for (size_t x = 1; x <= ns->world_width; ++x) {
                ns->u_prev[y][x] =
                        (ns->v_prev[y][x] + ns->u_prev[y][x - 1] + ns->u_prev[y][x + 1] + ns->u_prev[y - 1][x] +
                         ns->u_prev[y + 1][x]) / 4;
            }
        }

        set_bnd(ns, 0, ns->u_prev);
    }

    for (size_t y = 1; y <= ns->world_height; ++y) {
        for (size_t x = 1; x <= ns->world_width; ++x) {
            ns->u[y][x] -= 0.5 * (ns->u_prev[y][x + 1] - ns->u_prev[y][x - 1]) / h;
            ns->v[y][x] -= 0.5 * (ns->u_prev[y + 1][x] - ns->u_prev[y - 1][x]) / h;
        }
    }

    set_bnd(ns, 1, ns->u);
    set_bnd(ns, 2, ns->v);
}

static void advect(const navier_stokes_t *ns, size_t bounds, double **d, double **d0, double **u, double **v) {
    // TODO controlla source dato che utilizza N
    double dt0_width = ns->time_step * (double) ns->world_width;
    double dt0_height = ns->time_step * (double) ns->world_height;

    // TODO Cambia i nomi please
    for (size_t _y = 1; _y <= ns->world_height; ++_y) {
        for (size_t _x = 1; _x <= ns->world_width; ++_x) {
            double x = (double) _x - dt0_width * u[_y][_x];
            double y = (double) _y - dt0_height * v[_y][_x];

            // Check x
            if (x < 0.5)
                x = 0.5;
            if (x > (double) ns->world_width + 0.5)
                x = (double) ns->world_width + 0.5;
            size_t x0 = (size_t) x;
            size_t x1 = x0 + 1;

            // Check y
            if (y < 0.5)
                y = 0.5;
            if (y > (double) ns->world_height + 0.5)
                y = (double) ns->world_height + 0.5;
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

    set_bnd(ns, bounds, d);
}

static void set_bnd(const navier_stokes_t *ns, size_t bounds, double **target) {
    for (size_t y = 1; y <= ns->world_height; ++y) {
        for (size_t x = 1; x <= ns->world_width; ++x) {
            target[y][0] = (bounds == 1) ? -target[y][1] : target[y][1];
            target[y][ns->world_width + 1] = bounds == 1 ? -target[y][ns->world_width] : target[y][ns->world_width];
            target[0][x] = bounds == 2 ? -target[1][x] : target[1][x];
            target[ns->world_height + 1][x] = bounds == 2 ? -target[ns->world_height][x] : target[ns->world_height][x];
        }
    }

    target[0][0] = 0.5 * (target[0][1] + target[1][0]);
    target[ns->world_height + 1][0] = 0.5 * (target[ns->world_height + 1][1] + target[ns->world_height][0]);
    target[0][ns->world_width + 1] = 0.5 * (target[0][ns->world_width] + target[1][ns->world_width + 1]);
    target[ns->world_height + 1][ns->world_width + 1] =
            0.5 * (target[ns->world_height + 1][ns->world_width] + target[ns->world_height][ns->world_width + 1]);
}
