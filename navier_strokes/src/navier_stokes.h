#ifndef _NAVIER_STOKES_H
#define _NAVIER_STOKES_H

#include <stddef.h>
#include <stdbool.h>

typedef struct ns_t ns_t;

typedef struct ns_cell_t {
    double *u;
    double *v;
    double *density;
} ns_cell_t;

typedef struct ns_world_t {
    size_t world_width;
    size_t world_width_bounds;
    size_t world_height;
    size_t world_height_bounds;
    ns_cell_t **world;
} ns_world_t;

ns_t *ns_create(size_t world_width, size_t world_height,
                double viscosity, double density, double diffusion,
                double time_step);

void ns_free(ns_t *ns);

void ns_tick(ns_t *ns);

bool ns_increase_density(ns_t *ns, size_t x, size_t y);

bool ns_apply_force(ns_t *ns, size_t x, size_t y, double v_x, double v_y);

ns_world_t *ns_get_world(const ns_t *ns);

void ns_free_world(ns_world_t *world);

#endif
