#include "ns/solver.h"
#include "ns/config.h"
#include <stdlib.h>
#include <stdio.h>

// Data wrapper
typedef struct ns_t {
    // World
    uint64_t world_width;
    uint64_t world_width_bounds;
    uint64_t world_height;
    uint64_t world_height_bounds;

    // Fluid
    double viscosity;
    double density;
    double diffusion;

    // Time
    double time_step;

    // World data
    double **u;
    double **u_prev;
    double **v;
    double **v_prev;
    double **dense;
    double **dense_prev;
} ns_t;

/**
 * Private definitions
 */
static void ns_velocity_step(ns_t *ns);

static void ns_density_step(ns_t *ns);

static void ns_add_sources_to_targets(const ns_t *ns);

static void
ns_diffuse(const ns_t *ns, uint64_t bounds, double diffusion_value, double **target, const double **source);

static void ns_project(ns_t *ns);

static void ns_advect(const ns_t *ns, uint64_t bounds, double **d, double **d0, double **u, double **v);

static void ns_set_bounds(const ns_t *ns, uint64_t bounds, double **target);

static void ns_swap_matrix(double ***x, double ***y);

static bool is_valid_coordinate(const ns_t *ns, uint64_t x, uint64_t y);
/**
 * END Private definitions
 */

/**
 * Public
 */
ns_t *ns_create(uint64_t world_width, uint64_t world_height,
                double viscosity, double density, double diffusion,
                double time_step) {
    uint64_t i;
    bool error = false;
    ns_t *ns = NULL;

    ns = (ns_t *) malloc(sizeof(ns_t));
    if (ns == NULL) return NULL;

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

    // Allocate world data
    ns->u = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    ns->u_prev = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    ns->v = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    ns->v_prev = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    ns->dense = (double **) calloc(ns->world_height_bounds, sizeof(double *));
    ns->dense_prev = (double **) calloc(ns->world_height_bounds, sizeof(double *));

    if (ns->u == NULL || ns->u_prev == NULL
        || ns->v == NULL || ns->v_prev == NULL
        || ns->dense == NULL || ns->dense_prev == NULL) {
        error = true;
    }

    if (!error) {
#pragma omp parallel for \
    schedule(DEFAULT_OPEN_MP_SCHEDULE) \
    default(none) private(i) shared(ns, error)
        for (i = 0; i < ns->world_height_bounds; ++i) {
            if (error) continue;

            ns->u[i] = (double *) calloc(ns->world_width_bounds, sizeof(double));
            ns->u_prev[i] = (double *) calloc(ns->world_width_bounds, sizeof(double));
            ns->v[i] = (double *) calloc(ns->world_width_bounds, sizeof(double));
            ns->v_prev[i] = (double *) calloc(ns->world_width_bounds, sizeof(double));
            ns->dense[i] = (double *) calloc(ns->world_width_bounds, sizeof(double));
            ns->dense_prev[i] = (double *) calloc(ns->world_width_bounds, sizeof(double));

            if (ns->u[i] == NULL || ns->u_prev[i] == NULL
                || ns->v[i] == NULL || ns->v_prev[i] == NULL
                || ns->dense[i] == NULL || ns->dense_prev[i] == NULL) {
#pragma omp critical
                error = true;
            }
        }
    }

    if (error) {
        ns_free(ns);
        return NULL;
    }

    return ns;
}

void ns_free(ns_t *ns) {
    if (ns == NULL) return;
    uint64_t i;

#pragma omp parallel for \
    schedule(DEFAULT_OPEN_MP_SCHEDULE) \
    default(none) private(i) shared(ns)
    for (i = 0; i < ns->world_height_bounds; ++i) {
        if (ns->u != NULL)
            free(ns->u[i]);
        if (ns->u_prev != NULL)
            free(ns->u_prev[i]);
        if (ns->v != NULL)
            free(ns->v[i]);
        if (ns->v_prev != NULL)
            free(ns->v_prev[i]);
        if (ns->dense != NULL)
            free(ns->dense[i]);
        if (ns->dense_prev != NULL)
            free(ns->dense_prev[i]);
    }

    free(ns->u);
    free(ns->u_prev);
    free(ns->v);
    free(ns->v_prev);
    free(ns->dense);
    free(ns->dense_prev);

    free(ns);
}

void ns_tick(ns_t *ns) {
    ns_velocity_step(ns);
    ns_density_step(ns);
}

bool ns_increase_density(ns_t *ns, uint64_t x, uint64_t y) {
    bool status = false;

    // Fix due to bounds
    x += 1;
    y += 1;

    if (!is_valid_coordinate(ns, x, y))
        fprintf(stderr, "Invalid increase_density coordinates {x: %ld, y: %ld}\n", x, y);
    else status = true;

    if (status)
        ns->dense[y][x] += ns->density;

    return status;
}

bool ns_apply_force(ns_t *ns, uint64_t x, uint64_t y, double v_x, double v_y) {
    bool status = false;

    // Fix due to bounds
    x += 1;
    y += 1;

    if (!is_valid_coordinate(ns, x, y))
        fprintf(stderr, "Invalid apply_force coordinates {x: %ld, y: %ld}\n", x, y);
    else if (v_x > NS_MAX_FORCE_VELOCITY || v_y > NS_MAX_FORCE_VELOCITY)
        fprintf(stdout, "Invalid apply_force velocity {v_x: %lf, v_y: %lf}\n", v_x, v_y);
    else status = true;

    if (status) {
        ns->u[y][x] = v_x != 0 ? v_x : ns->u[y][x];
        ns->v[y][x] = v_y != 0 ? v_y : ns->v[y][x];
    }

    return status;
}

ns_world_t *ns_get_world(const ns_t *ns) {
    uint64_t i, x, y;
    ns_world_t *world = (ns_world_t *) malloc(sizeof(ns_world_t));

    world->world_width = ns->world_width;
    world->world_width_bounds = ns->world_width_bounds;
    world->world_height = ns->world_height;
    world->world_height_bounds = ns->world_height_bounds;

    world->world = (ns_cell_t **) calloc(ns->world_height_bounds, sizeof(ns_cell_t *));

#pragma omp parallel \
default(none) private(i) shared(ns, world)
    {
#pragma omp for \
        schedule(DEFAULT_OPEN_MP_SCHEDULE)
        for (i = 0; i < ns->world_height_bounds; ++i)
            world->world[i] = (ns_cell_t *) calloc(ns->world_width_bounds, sizeof(ns_cell_t));

#pragma omp for collapse(2) \
        schedule(DEFAULT_OPEN_MP_SCHEDULE)
        for (y = 0; y < ns->world_height_bounds; ++y) {
            for (x = 0; x < ns->world_width_bounds; ++x) {
                ns_cell_t cell;
                cell.u = &ns->u[y][x];
                cell.v = &ns->v[y][x];
                cell.density = &ns->dense[y][x];

                world->world[y][x] = cell;
            }
        }
    }

    return world;
}

void ns_free_world(ns_world_t *world) {
    uint64_t i;

#pragma omp parallel for \
    schedule(DEFAULT_OPEN_MP_SCHEDULE) \
    default(none) private(i) shared(world)
    for (i = 0; i < world->world_height_bounds; ++i) {
        free(world->world[i]);
    }

    free(world->world);
    free(world);
}
/**
 * END Public
 */

/**
 * Private
 */
static void ns_velocity_step(ns_t *ns) {
    ns_add_sources_to_targets(ns);

    ns_swap_matrix(&ns->u_prev, &ns->u);
    ns_diffuse(ns, 1, ns->viscosity, ns->u, (const double **) ns->u_prev);

    ns_swap_matrix(&ns->v_prev, &ns->v);
    ns_diffuse(ns, 2, ns->viscosity, ns->v, (const double **) ns->v_prev);
    ns_project(ns);
    ns_swap_matrix(&ns->u_prev, &ns->u);
    ns_swap_matrix(&ns->v_prev, &ns->v);
    ns_advect(ns, 1, ns->u, ns->u_prev, ns->u_prev, ns->v_prev);
    ns_advect(ns, 2, ns->v, ns->v_prev, ns->u_prev, ns->v_prev);
    ns_project(ns);
}

static void ns_density_step(ns_t *ns) {
    ns_swap_matrix(&ns->dense_prev, &ns->dense);
    ns_diffuse(ns, 0, ns->diffusion, ns->dense, (const double **) ns->dense_prev);
    ns_swap_matrix(&ns->dense_prev, &ns->dense);
    ns_advect(ns, 0, ns->dense, ns->dense_prev, ns->u, ns->v);
}

static void ns_add_sources_to_targets(const ns_t *ns) {
    uint64_t x, y;

#pragma omp parallel for collapse(2) \
    schedule(DEFAULT_OPEN_MP_SCHEDULE) \
    default(none) private(y, x) shared(ns)
    for (y = 0; y < ns->world_height_bounds; ++y) {
        for (x = 0; x < ns->world_width_bounds; ++x) {
            ns->u[y][x] += ns->time_step * ns->u_prev[y][x];
            ns->v[y][x] += ns->time_step * ns->v_prev[y][x];
        }
    }
}

static void
ns_diffuse(const ns_t *ns, uint64_t bounds, double diffusion_value, double **target, const double **source) {
    const double a = ns->time_step * diffusion_value * (double) ns->world_width * (double) ns->world_height;

    for (uint64_t k = 0; k < 20; k++) {
        for (uint64_t y = 1; y <= ns->world_height; ++y) {
            for (uint64_t x = 1; x <= ns->world_width; ++x) {
                target[y][x] =
                        (source[y][x] + a * (target[y][x - 1] + target[y][x + 1] + target[y - 1][x] + target[y + 1][x]))
                        / (1 + 4 * a);
            }
        }

        ns_set_bounds(ns, bounds, target);
    }
}

static void ns_project(ns_t *ns) {
    uint64_t x, y;
    double h = 1.0 / (double) ns->world_width;

    for (y = 1; y <= ns->world_height; ++y) {
        for (x = 1; x <= ns->world_width; ++x) {
            ns->v_prev[y][x] = -0.5 * h
                               * (ns->u[y][x + 1] - ns->u[y][x - 1] + ns->v[y + 1][x] - ns->v[y - 1][x]);
            ns->u_prev[y][x] = 0;
        }
    }

    ns_set_bounds(ns, 0, ns->v_prev);
    ns_set_bounds(ns, 0, ns->u_prev);

    for (uint64_t k = 0; k < 20; k++) {
        for (y = 1; y <= ns->world_height; ++y) {
            for (x = 1; x <= ns->world_width; ++x) {
                ns->u_prev[y][x] =
                        (ns->v_prev[y][x]
                         + ns->u_prev[y][x - 1] + ns->u_prev[y][x + 1] + ns->u_prev[y - 1][x] + ns->u_prev[y + 1][x])
                        / 4;
            }
        }

        ns_set_bounds(ns, 0, ns->u_prev);
    }

#pragma omp parallel for collapse(2) \
    schedule(DEFAULT_OPEN_MP_SCHEDULE) \
    default(none) private(y, x) shared(ns, h)
    for (y = 1; y <= ns->world_height; ++y) {
        for (x = 1; x <= ns->world_width; ++x) {
            ns->u[y][x] -= 0.5 * (ns->u_prev[y][x + 1] - ns->u_prev[y][x - 1]) / h;
            ns->v[y][x] -= 0.5 * (ns->u_prev[y + 1][x] - ns->u_prev[y - 1][x]) / h;
        }
    }

    ns_set_bounds(ns, 1, ns->u);
    ns_set_bounds(ns, 2, ns->v);
}

static void ns_advect(const ns_t *ns, uint64_t bounds, double **d, double **d0, double **u, double **v) {
    uint64_t x, y, x0, x1, y0, y1;
    double xx, yy, s0, s1, t0, t1;
    double dt0_width = ns->time_step * (double) ns->world_width;
    double dt0_height = ns->time_step * (double) ns->world_height;

#pragma omp parallel for collapse(2) \
    schedule(DEFAULT_OPEN_MP_SCHEDULE) \
    default(none) private(y, x, yy, xx, x0, x1, y0, y1, s0, s1, t0, t1) shared(ns, dt0_width, dt0_height, u, v, d, d0)
    for (y = 1; y <= ns->world_height; ++y) {
        for (x = 1; x <= ns->world_width; ++x) {
            xx = (double) x - dt0_width * u[y][x];
            yy = (double) y - dt0_height * v[y][x];

            // Check xx
            if (xx < 0.5)
                xx = 0.5;
            if (xx > (double) ns->world_width + 0.5)
                xx = (double) ns->world_width + 0.5;
            x0 = (uint64_t) xx;
            x1 = x0 + 1;

            // Check yy
            if (yy < 0.5)
                yy = 0.5;
            if (yy > (double) ns->world_height + 0.5)
                yy = (double) ns->world_height + 0.5;
            y0 = (uint64_t) yy;
            y1 = y0 + 1;


            s1 = xx - (double) x0;
            s0 = 1 - s1;
            t1 = yy - (double) y0;
            t0 = 1 - t1;

            d[y][x] = s0 * (t0 * d0[y0][x0] + t1 * d0[y1][x0])
                      + s1 * (t0 * d0[y0][x1] + t1 * d0[y1][x1]);
        }
    }

    ns_set_bounds(ns, bounds, d);
}

static void ns_set_bounds(const ns_t *ns, uint64_t bounds, double **target) {
    uint64_t y;
    uint64_t x;

#pragma omp parallel for collapse(2) \
    schedule(DEFAULT_OPEN_MP_SCHEDULE) \
    default(none) private(y, x) shared(ns, target, bounds)
    for (y = 1; y <= ns->world_height; ++y) {
        for (x = 1; x <= ns->world_width; ++x) {
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

static void ns_swap_matrix(double ***x, double ***y) {
    double **tmp = *x;
    *x = *y;
    *y = tmp;
}

static bool is_valid_coordinate(const ns_t *ns, uint64_t x, uint64_t y) {
    return x >= 0 && x < ns->world_width_bounds
           && y >= 0 && y < ns->world_height_bounds;
}

/**
* END Private
*/
