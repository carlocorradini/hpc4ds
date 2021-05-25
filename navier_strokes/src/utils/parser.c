#include "ns/utils/parser.h"
#include <stdlib.h>
#include <stdbool.h>
#include <cJSON.h>

/**
 * Private definitions
 */
static bool ns_parse_simulation_check_and_assign_time_step(const cJSON *time_step_json, double *time_step);

static bool ns_parse_simulation_check_and_assign_ticks(const cJSON *ticks_json, uint64_t *ticks);

static bool ns_parse_simulation_check_and_assign_world(const cJSON *world_json, ns_parse_simulation_world_t *world);

static bool ns_parse_simulation_check_and_assign_fluid(const cJSON *fluid_json, ns_parse_simulation_fluid_t *fluid);

static bool ns_parse_simulation_check_and_assign_mod(const cJSON *mod_json, ns_parse_simulation_mod_t *mod);

static bool ns_parse_simulation_check_and_assign_mods(const cJSON *mods_json, ns_simulation_t *simulation);

static void *ns_parse_simulation_error(cJSON *file_json, ns_simulation_t *simulation);

static void *ns_parse_simulations_error(cJSON *file_json, ns_simulations_t *simulations);
/**
 * END Private definitions
 */

/**
 * Public
 */
ns_simulation_t *ns_parse_simulation(const char *const text) {
    if (text == NULL) return NULL;
    ns_simulation_t *simulation = NULL;
    cJSON *simulation_json = NULL;

    simulation = (ns_simulation_t *) malloc(sizeof(ns_simulation_t));
    if (simulation == NULL) return ns_parse_simulation_error(simulation_json, simulation);

    simulation_json = cJSON_Parse(text);
    if (simulation_json == NULL) return ns_parse_simulation_error(simulation_json, simulation);

    // Check & Assign
    if (!(ns_parse_simulation_check_and_assign_time_step(
            cJSON_GetObjectItemCaseSensitive(simulation_json, "time_step"), &simulation->time_step)
          && ns_parse_simulation_check_and_assign_ticks(
            cJSON_GetObjectItemCaseSensitive(simulation_json, "ticks"), &simulation->ticks)
          && ns_parse_simulation_check_and_assign_world(
            cJSON_GetObjectItemCaseSensitive(simulation_json, "world"), &simulation->world)
          && ns_parse_simulation_check_and_assign_fluid(
            cJSON_GetObjectItemCaseSensitive(simulation_json, "fluid"), &simulation->fluid)
          && ns_parse_simulation_check_and_assign_mods(
            cJSON_GetObjectItemCaseSensitive(simulation_json, "mods"), simulation)
    ))
        return ns_parse_simulation_error(simulation_json, simulation);

    cJSON_Delete(simulation_json);
    return simulation;

}

ns_simulations_t *ns_parse_simulations(const char *const text) {
    if (text == NULL) return NULL;
    ns_simulations_t *simulations = NULL;
    cJSON *text_json = NULL;
    const cJSON *simulation_json = NULL;
    const cJSON *simulations_json = NULL;

    simulations = (ns_simulations_t *) malloc(sizeof(ns_simulations_t));
    if (simulations == NULL) return ns_parse_simulations_error(text_json, simulations);

    text_json = cJSON_Parse(text);
    if (text_json == NULL) return ns_parse_simulations_error(text_json, simulations);

    simulations_json = cJSON_GetObjectItemCaseSensitive(text_json, "simulations");
    if (!cJSON_IsArray(simulations_json)) return ns_parse_simulations_error(text_json, simulations);

    simulations->simulations_length = (uint64_t) cJSON_GetArraySize(simulations_json);
    simulations->simulations = (ns_simulation_t **) calloc(simulations->simulations_length,
                                                           sizeof(ns_simulation_t *));
    if (simulations->simulations == NULL) return ns_parse_simulations_error(text_json, simulations);

    uint64_t index = 0;
    cJSON_ArrayForEach(simulation_json, simulations_json) {
        char *simulation_text = cJSON_Print(simulation_json);
        ns_simulation_t *simulation = ns_parse_simulation(simulation_text);
        free(simulation_text);

        if (simulation == NULL)
            return ns_parse_simulations_error(text_json, simulations);

        simulations->simulations[index] = simulation;
        index += 1;
    }

    cJSON_Delete(text_json);
    return simulations;
}

void ns_parse_simulation_free(ns_simulation_t *simulation) {
    if (simulation != NULL && simulation->mods != NULL) {
        for (uint64_t i_m = 0; i_m < simulation->mods_length; ++i_m) {
            ns_parse_simulation_mod_t *mod = simulation->mods[i_m];

            if (mod != NULL && mod->densities != NULL) {
                for (uint64_t i_d = 0; i_d < mod->densities_length; ++i_d) {
                    ns_parse_simulation_mods_density_t *density = mod->densities[i_d];
                    free(density);
                }
                free(mod->densities);
            }

            if (mod != NULL && mod->forces != NULL) {
                for (uint64_t i_f = 0; i_f < mod->densities_length; ++i_f) {
                    ns_parse_simulation_mods_force_t *force = mod->forces[i_f];
                    free(force);
                }
                free(mod->forces);
            }

            free(mod);
        }

        free(simulation->mods);
    }

    free(simulation);
}

void ns_parse_simulations_free(ns_simulations_t *simulations) {
    if (simulations != NULL && simulations->simulations != NULL) {
        for (uint64_t i_s = 0; i_s < simulations->simulations_length; ++i_s) {
            ns_parse_simulation_free(simulations->simulations[i_s]);
        }

        free(simulations->simulations);
    }

    free(simulations);
}
/**
 * END Public
 */

/**
 * Private
 */
static bool ns_parse_simulation_check_and_assign_time_step(const cJSON *const time_step_json, double *time_step) {
    if (time_step_json == NULL || time_step == NULL) return false;

    if (!(cJSON_IsNumber(time_step_json) && time_step_json->valuedouble > 0.0))
        return false;

    *time_step = time_step_json->valuedouble;

    return true;
}

static bool ns_parse_simulation_check_and_assign_ticks(const cJSON *const ticks_json, uint64_t *ticks) {
    if (ticks_json == NULL || ticks == NULL) return false;

    if (!(cJSON_IsNumber(ticks_json) && ticks_json->valueint > 0))
        return false;

    *ticks = (uint64_t) ticks_json->valueint;

    return true;
}

static bool
ns_parse_simulation_check_and_assign_world(const cJSON *const world_json, ns_parse_simulation_world_t *world) {
    if (world_json == NULL || world == NULL) return false;

    const cJSON *width_json = NULL;
    const cJSON *height_json = NULL;

    width_json = cJSON_GetObjectItemCaseSensitive(world_json, "width");
    height_json = cJSON_GetObjectItemCaseSensitive(world_json, "height");

    if (!(cJSON_IsNumber(width_json) && width_json->valueint > 0
          && cJSON_IsNumber(height_json) && height_json->valueint > 0
    ))
        return false;

    world->width = (uint64_t) width_json->valueint;
    world->height = (uint64_t) height_json->valueint;

    return true;
}

static bool
ns_parse_simulation_check_and_assign_fluid(const cJSON *const fluid_json, ns_parse_simulation_fluid_t *fluid) {
    if (fluid_json == NULL || fluid == NULL) return false;

    const cJSON *viscosity_json = NULL;
    const cJSON *density_json = NULL;
    const cJSON *diffusion_json = NULL;

    viscosity_json = cJSON_GetObjectItemCaseSensitive(fluid_json, "viscosity");
    density_json = cJSON_GetObjectItemCaseSensitive(fluid_json, "density");
    diffusion_json = cJSON_GetObjectItemCaseSensitive(fluid_json, "diffusion");

    if (!(cJSON_IsNumber(viscosity_json) && viscosity_json->valuedouble > 0
          && cJSON_IsNumber(density_json) && density_json->valuedouble > 0
          && cJSON_IsNumber(diffusion_json) && diffusion_json->valuedouble > 0
    ))
        return false;

    fluid->viscosity = viscosity_json->valuedouble;
    fluid->density = density_json->valuedouble;
    fluid->diffusion = diffusion_json->valuedouble;

    return true;
}


#include <ns/utils/logger.h>
static bool ns_parse_simulation_check_and_assign_mod(const cJSON *const mod_json, ns_parse_simulation_mod_t *mod) {
    if (mod_json == NULL || mod == NULL) return false;

    const cJSON *tick_json = NULL;
    const cJSON *densities_json = NULL;
    const cJSON *forces_json = NULL;

    tick_json = cJSON_GetObjectItemCaseSensitive(mod_json, "tick");
    densities_json = cJSON_GetObjectItemCaseSensitive(mod_json, "densities");
    forces_json = cJSON_GetObjectItemCaseSensitive(mod_json, "forces");

    if (!(cJSON_IsNumber(tick_json) && tick_json->valueint >= 0
          && (densities_json == NULL || cJSON_IsNull(densities_json) || cJSON_IsArray(densities_json))
          && (forces_json == NULL || cJSON_IsNull(forces_json) || cJSON_IsArray(forces_json))
    ))
        return false;

    mod->tick = (uint64_t) tick_json->valueint;

    if (densities_json == NULL || cJSON_IsNull(densities_json)) {
        mod->densities_length = 0;
        mod->densities = NULL;
    } else {
        const cJSON *density_json = NULL;

        mod->densities_length = (uint64_t) cJSON_GetArraySize(densities_json);
        mod->densities = (ns_parse_simulation_mods_density_t **) calloc(mod->densities_length,
                                                                        sizeof(ns_parse_simulation_mods_density_t *));
        uint64_t index = 0;
        cJSON_ArrayForEach(density_json, densities_json) {
            ns_parse_simulation_mods_density_t *density = NULL;
            const cJSON *x = NULL;
            const cJSON *y = NULL;

            density = (ns_parse_simulation_mods_density_t *) malloc(sizeof(ns_parse_simulation_mods_density_t));
            if (density == NULL) return false;

            x = cJSON_GetObjectItemCaseSensitive(density_json, "x");
            y = cJSON_GetObjectItemCaseSensitive(density_json, "y");

            if (!(cJSON_IsNumber(x) && x->valueint >= 0
                  && cJSON_IsNumber(y) && y->valueint >= 0
            ))
                return false;

            density->x = (uint64_t) x->valueint;
            density->y = (uint64_t) y->valueint;

            mod->densities[index] = density;
            index += 1;
        }
    }

    if (forces_json == NULL || cJSON_IsNull(forces_json)) {
        mod->forces_length = 0;
        mod->forces = NULL;
    } else {
        const cJSON *force_json = NULL;

        mod->forces_length = (uint64_t) cJSON_GetArraySize(forces_json);
        mod->forces = (ns_parse_simulation_mods_force_t **) calloc(mod->forces_length,
                                                                   sizeof(ns_parse_simulation_mods_force_t *));
        uint64_t index = 0;
        cJSON_ArrayForEach(force_json, forces_json) {
            ns_parse_simulation_mods_force_t *force = NULL;
            const cJSON *x = NULL;
            const cJSON *y = NULL;
            const cJSON *velocity = NULL;
            const cJSON *velocity_x = NULL;
            const cJSON *velocity_y = NULL;

            force = (ns_parse_simulation_mods_force_t *) malloc(sizeof(ns_parse_simulation_mods_force_t));
            if (force == NULL) return false;

            velocity = cJSON_GetObjectItemCaseSensitive(force_json, "velocity");
            if (!cJSON_IsObject(velocity)) return false;

            x = cJSON_GetObjectItemCaseSensitive(force_json, "x");
            y = cJSON_GetObjectItemCaseSensitive(force_json, "y");
            velocity_x = cJSON_GetObjectItemCaseSensitive(velocity, "x");
            velocity_y = cJSON_GetObjectItemCaseSensitive(velocity, "y");

            if (!(cJSON_IsNumber(x) && x->valueint >= 0
                  && cJSON_IsNumber(y) && y->valueint >= 0
                  && cJSON_IsNumber(velocity_x) && velocity_x->valuedouble >= 0
                  && cJSON_IsNumber(velocity_y) && velocity_y->valuedouble >= 0
            ))
                return false;

            force->x = (uint64_t) x->valueint;
            force->y = (uint64_t) y->valueint;
            force->velocity.x = velocity_x->valuedouble;
            force->velocity.y = velocity_y->valuedouble;

            mod->forces[index] = force;
            index += 1;
        }
    }

    return true;
}

static bool ns_parse_simulation_check_and_assign_mods(const cJSON *const mods_json, ns_simulation_t *simulation) {
    if (simulation == NULL) return false;
    if (mods_json == NULL) {
        simulation->mods_length = 0;
        simulation->mods = NULL;
        return true;
    }
    if (!cJSON_IsArray(mods_json)) return false;

    const cJSON *mod_json = NULL;

    simulation->mods_length = (uint64_t) cJSON_GetArraySize(mods_json);
    simulation->mods = (ns_parse_simulation_mod_t **) calloc(simulation->mods_length,
                                                             sizeof(ns_parse_simulation_mod_t *));
    if (simulation->mods == NULL) return false;

    uint64_t index = 0;
    cJSON_ArrayForEach(mod_json, mods_json) {
        ns_parse_simulation_mod_t *mod = NULL;

        mod = (ns_parse_simulation_mod_t *) malloc(sizeof(ns_parse_simulation_mod_t));
        if (mod == NULL) return false;

        if (!ns_parse_simulation_check_and_assign_mod(mod_json, mod)) return false;

        simulation->mods[index] = mod;

        index += 1;
    }

    return true;
}

static void *ns_parse_simulation_error(cJSON *file_json, ns_simulation_t *simulation) {
    cJSON_Delete(file_json);
    ns_parse_simulation_free(simulation);
    return NULL;
}

static void *ns_parse_simulations_error(cJSON *file_json, ns_simulations_t *simulations) {
    cJSON_Delete(file_json);
    ns_parse_simulations_free(simulations);
    return NULL;
}
/**
* END Private
*/
