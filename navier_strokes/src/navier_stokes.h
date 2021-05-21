#ifndef _NAVIER_STOKES_H
#define _NAVIER_STOKES_H

#include <stddef.h>
#include <stdbool.h>

// Maximum force velocity
#define NS_MAX_FORCE_VELOCITY 120.0

// Data wrapper (opaque)
typedef struct ns_t ns_t;

// Single cell containing u,v and density
typedef struct ns_cell_t {
    double *u;
    double *v;
    double *density;
} ns_cell_t;

// World data snapshot
typedef struct ns_world_t {
    size_t world_width;
    size_t world_width_bounds;
    size_t world_height;
    size_t world_height_bounds;
    ns_cell_t **world;
} ns_world_t;

/**
 * Create a new Navier Stokes world scenario.
 * Remember to free with ns_free.
 *
 * @param world_width World width
 * @param world_height World height
 * @param viscosity Fluid viscosity
 * @param density Fluid density
 * @param diffusion Fluid diffusion
 * @param time_step Tick time step
 * @return Reference to Navier Stokes data wrapper
 */
ns_t *ns_create(size_t world_width, size_t world_height,
                double viscosity, double density, double diffusion,
                double time_step);

/**
 * Free the Navier Stokes world scenario.
 *
 * @param ns Reference to Navier Stokes data wrapper
 */
void ns_free(ns_t *ns);

/**
 * Do a time tick of duration time step.
 *
 * @param ns Reference to Navier Stokes data wrapper
 */
void ns_tick(ns_t *ns);

/**
 * Increase fluid density in cell (x, y).
 *
 * @param ns Reference to Navier Stokes data wrapper
 * @param x X coordinate
 * @param y Y coordinate
 * @return true if increased, false otherwise
 */
bool ns_increase_density(ns_t *ns, size_t x, size_t y);

/**
 * Apply a force in cell (x, y) with velocity (v_x, v_y).
 *
 * @param ns Reference to Navier Stokes data wrapper
 * @param x X coordinate
 * @param y Y coordinate
 * @param v_x X velocity
 * @param v_y Y velocity
 * @return true if applied, false otherwise
 */
bool ns_apply_force(ns_t *ns, size_t x, size_t y, double v_x, double v_y);

/**
 * Create a Navier Stokes world snapshot.
 * Remember to free with ns_free_world.
 *
 * @param ns Reference to Navier Stokes data wrapper
 * @return Reference to Navier Stokes world snapshot data
 */
ns_world_t *ns_get_world(const ns_t *ns);

/**
 *  Free the Navier Stokes world snapshot.
 *
 * @param world Reference to Navier Stokes world snapshot data
 */
void ns_free_world(ns_world_t *world);

#endif
