#ifndef _NAVIER_STOKES_H
#define _NAVIER_STOKES_H

#include <stddef.h>

typedef struct navier_stokes_t navier_stokes_t;

navier_stokes_t *ns_create(size_t world_width, size_t world_height,
                           double viscosity, double density, double diffusion,
                           double time_step);

void tick(navier_stokes_t *ns);

void increase_density(navier_stokes_t *ns, size_t x, size_t y);

void apply_force(navier_stokes_t *ns, size_t cellX, size_t cellY, double vX, double vY);

const double **get_world(const navier_stokes_t *ns);

#endif
