#include "ns/utils/stringify.h"
#include <cJSON.h>

/**
 * Private definitions
 */
static void *ns_stringify_simulation_error(cJSON *simulation_json);
/**
 * END Private definitions
 */

/**
 * Public
 */
char *ns_stringify_simulation(const ns_simulation_t *const simulation) {
    if (simulation == NULL) return NULL;
    char *text = NULL;
    cJSON *simulation_json = NULL;
    cJSON *world_json = NULL;
    cJSON *fluid_json = NULL;
    cJSON *mods_json = NULL;

    simulation_json = cJSON_CreateObject();

    if (cJSON_AddNumberToObject(simulation_json, "time_step", simulation->time_step) == NULL
        || cJSON_AddNumberToObject(simulation_json, "ticks", (double) simulation->ticks) == NULL)
        return ns_stringify_simulation_error(simulation_json);

    world_json = cJSON_AddObjectToObject(simulation_json, "world");
    if (world_json == NULL) return ns_stringify_simulation_error(simulation_json);
    if (cJSON_AddNumberToObject(world_json, "width", (double) simulation->world.width) == NULL
        || cJSON_AddNumberToObject(world_json, "height", (double) simulation->world.height) == NULL)
        return ns_stringify_simulation_error(simulation_json);

    fluid_json = cJSON_AddObjectToObject(simulation_json, "fluid");
    if (fluid_json == NULL) return ns_stringify_simulation_error(simulation_json);
    if (cJSON_AddNumberToObject(fluid_json, "viscosity", simulation->fluid.viscosity) == NULL
        || cJSON_AddNumberToObject(fluid_json, "density", simulation->fluid.density) == NULL
        || cJSON_AddNumberToObject(fluid_json, "diffusion", simulation->fluid.diffusion) == NULL)
        return ns_stringify_simulation_error(simulation_json);

    mods_json = cJSON_AddArrayToObject(simulation_json, "mods");
    if (mods_json == NULL) return ns_stringify_simulation_error(simulation_json);
    if (simulation->mods != NULL && simulation->mods_length > 0) {
        for (uint64_t i_m = 0; i_m < simulation->mods_length; ++i_m) {
            const ns_parse_simulation_mod_t *const mod = simulation->mods[i_m];
            cJSON *mod_json;
            cJSON *densities_json;
            cJSON *forces_json;

            mod_json = cJSON_CreateObject();
            densities_json = cJSON_AddArrayToObject(mod_json, "densities");
            forces_json = cJSON_AddArrayToObject(mod_json, "forces");

            if (cJSON_AddNumberToObject(mod_json, "tick", (double) mod->tick) == NULL
                || densities_json == NULL
                || forces_json == NULL)
                return ns_stringify_simulation_error(simulation_json);

            if (mod->densities != NULL && mod->densities_length > 0) {
                for (uint64_t i_d = 0; i_d < mod->densities_length; ++i_d) {
                    const ns_parse_simulation_mods_density_t *const density = mod->densities[i_d];
                    cJSON *density_json = NULL;

                    density_json = cJSON_CreateObject();
                    if (cJSON_AddNumberToObject(density_json, "x", (double) density->x) == NULL
                        || cJSON_AddNumberToObject(density_json, "y", (double) density->y) == NULL)
                        return ns_stringify_simulation_error(simulation_json);

                    cJSON_AddItemToArray(densities_json, density_json);
                }
            }

            if (mod->forces != NULL && mod->forces_length > 0) {
                for (uint64_t i_f = 0; i_f < mod->forces_length; ++i_f) {
                    const ns_parse_simulation_mods_force_t *const force = mod->forces[i_f];
                    cJSON *force_json = NULL;
                    cJSON *velocity_json = NULL;

                    force_json = cJSON_CreateObject();
                    velocity_json = cJSON_AddObjectToObject(force_json, "velocity");

                    if (velocity_json == NULL
                        || cJSON_AddNumberToObject(force_json, "x", (double) force->x) == NULL
                        || cJSON_AddNumberToObject(force_json, "y", (double) force->y) == NULL
                        || cJSON_AddNumberToObject(velocity_json, "x", force->velocity.x) == NULL
                        || cJSON_AddNumberToObject(velocity_json, "y", force->velocity.y) == NULL)
                        return ns_stringify_simulation_error(simulation_json);

                    cJSON_AddItemToArray(forces_json, force_json);
                }
            }

            cJSON_AddItemToArray(mods_json, mod_json);
        }
    }

    text = cJSON_Print(simulation_json);
    if (text == NULL) return ns_stringify_simulation_error(simulation_json);

    return text;
}
/**
 * END Public
 */

/**
 * Private
 */
static void *ns_stringify_simulation_error(cJSON *simulation_json) {
    cJSON_Delete(simulation_json);
    return NULL;
}
/**
* END Private
*/
