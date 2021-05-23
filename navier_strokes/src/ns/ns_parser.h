#ifndef _NS_PARSER_H
#define _NS_PARSER_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <cJSON.h>

typedef struct ns_parse_simulation_world_t {
    u_int64_t width;
    u_int64_t height;
} ns_parse_simulation_world_t;

typedef struct ns_parse_simulation_fluid_t {
    double viscosity;
    double density;
    double diffusion;
} ns_parse_simulation_fluid_t;

typedef struct ns_parse_simulation_mods_density_t {
    u_int64_t x;
    u_int64_t y;
} ns_parse_simulation_mods_density_t;

typedef struct ns_parse_simulation_mods_forces_velocity_t {
    double x;
    double y;
} ns_parse_simulation_mods_forces_velocity_t;

typedef struct ns_parse_simulation_mods_force_t {
    u_int64_t x;
    u_int64_t y;
    ns_parse_simulation_mods_forces_velocity_t velocity;
} ns_parse_simulation_mods_force_t;

typedef struct ns_parse_simulation_mod_t {
    u_int64_t tick;
    ns_parse_simulation_mods_density_t **densities;
    u_int64_t densities_length;
    ns_parse_simulation_mods_force_t **forces;
    u_int64_t forces_length;
} ns_parse_simulation_mod_t;

typedef struct ns_parse_simulation_t {
    double time_step;
    u_int64_t ticks;

    // World
    ns_parse_simulation_world_t world;

    // Fluid
    ns_parse_simulation_fluid_t fluid;

    // Mods
    ns_parse_simulation_mod_t **mods;
    u_int64_t mods_length;
} ns_parse_simulation_t;

typedef struct ns_parse_simulations_t {
    ns_parse_simulation_t **simulations;
    u_int64_t simulations_length;
} ns_parse_simulations_t;

/**
 * Parse text string into simulations struct.
 * Remember to free with ns_parse_simulations_free.
 *
 * @param text Text string to parse
 * @return Parsed JSON simulations struct, NULL otherwise
 */
ns_parse_simulations_t *ns_parse_simulations(const char *text);

/**
 * Free the parsed simulation.
 *
 * @param simulation Reference to parsed simulation
 */
void ns_parse_simulation_free(ns_parse_simulation_t *simulation);

/**
 * Free the parsed simulations.
 *
 * @param simulations Reference to parsed simulations
 */
void ns_parse_simulations_free(ns_parse_simulations_t *simulations);

#endif
