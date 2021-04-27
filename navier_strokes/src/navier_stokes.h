#ifndef _NAVIER_STOKES_H
#define _NAVIER_STOKES_H

#include <stddef.h>

typedef struct ns_t ns_t;

ns_t *ns_create(size_t world_width, size_t world_height,
                double viscosity, double density, double diffusion,
                double time_step);

void ns_tick(ns_t *ns);

void ns_increase_density(ns_t *ns, size_t x, size_t y);

void ns_apply_force(ns_t *ns, size_t cellX, size_t cellY, double vX, double vY);

const double **ns_get_world(const ns_t *ns);

#endif
