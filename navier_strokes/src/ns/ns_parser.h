#ifndef _NS_PARSER_H
#define _NS_PARSER_H

#include <stdint.h>

typedef struct ns_parse_simulation_world_t {
    uint64_t width;
    uint64_t height;
} ns_parse_simulation_world_t;

typedef struct ns_parse_simulation_fluid_t {
    double viscosity;
    double density;
    double diffusion;
} ns_parse_simulation_fluid_t;

typedef struct ns_parse_simulation_mods_density_t {
    uint64_t x;
    uint64_t y;
} ns_parse_simulation_mods_density_t;

typedef struct ns_parse_simulation_mods_forces_velocity_t {
    double x;
    double y;
} ns_parse_simulation_mods_forces_velocity_t;

typedef struct ns_parse_simulation_mods_force_t {
    uint64_t x;
    uint64_t y;
    ns_parse_simulation_mods_forces_velocity_t velocity;
} ns_parse_simulation_mods_force_t;

typedef struct ns_parse_simulation_mod_t {
    uint64_t tick;
    ns_parse_simulation_mods_density_t **densities;
    uint64_t densities_length;
    ns_parse_simulation_mods_force_t **forces;
    uint64_t forces_length;
} ns_parse_simulation_mod_t;

typedef struct ns_parse_simulation_t {
    double time_step;
    uint64_t ticks;

    // World
    ns_parse_simulation_world_t world;

    // Fluid
    ns_parse_simulation_fluid_t fluid;

    // Mods
    ns_parse_simulation_mod_t **mods;
    uint64_t mods_length;
} ns_parse_simulation_t;

typedef struct ns_parse_simulations_t {
    ns_parse_simulation_t **simulations;
    uint64_t simulations_length;
} ns_parse_simulations_t;

/**
 * Parse text string into simulation struct.
 * Remember to free with ns_parse_simulation_free.
 *
 * @param text Text string to parse
 * @return Parsed JSON simulation struct, NULL otherwise
 */
ns_parse_simulation_t *ns_parse_simulation(const char *text);

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
