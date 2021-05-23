#include <stdlib.h>
#include <stdio.h>
#include "ns/ns_solver.h"
#include "ns/ns_parser.h"

static ns_parse_simulation_mod_t *
find_mod_by_tick(const ns_parse_simulation_t *const simulation, u_int64_t tick) {
    if (simulation == NULL || simulation->mods == NULL
        || tick < 0 || tick > simulation->ticks - 1)
        return NULL;

    for (u_int64_t i = 0; i < simulation->mods_length; ++i) {
        ns_parse_simulation_mod_t *mod = simulation->mods[i];

        if (mod->tick == tick) return mod;
    }

    return NULL;
}

int main(void) {
    const char *string = "{\"simulations\":[{\"time_step\":0.01,\"ticks\":50,\"world\":{\"width\":100,\"height\":100},\"fluid\":{\"viscosity\":0.0001,\"density\":10,\"diffusion\":0.0001},\"mods\":[{\"tick\":0,\"densities\":[{\"x\":41,\"y\":41},{\"x\":65,\"y\":20},{\"x\":15,\"y\":20}],\"forces\":[{\"x\":41,\"y\":41,\"velocity\":{\"x\":0,\"y\":80}}]}]}]}";

    ns_parse_simulations_t *simulations = ns_parse_simulations(string);

    for (u_int64_t i_s = 0; i_s < simulations->simulations_length; ++i_s) {
        FILE *fp = NULL;
        ns_parse_simulation_t *simulation = NULL;
        ns_t *ns = NULL;
        ns_world_t *world = NULL;

        fp = fopen("output.txt", "w");
        simulation = simulations->simulations[i_s];
        ns = ns_create(simulation->world.width, simulation->world.height,
                       simulation->fluid.viscosity, simulation->fluid.density, simulation->fluid.diffusion,
                       simulation->time_step);
        world = ns_get_world(ns);

        for (u_int64_t tick = 0; tick < simulation->ticks; ++tick) {
            const ns_parse_simulation_mod_t *const mod = find_mod_by_tick(simulation, tick);

            if (mod != NULL) {
                for (u_int64_t i_d = 0; i_d < mod->densities_length; ++i_d) {
                    const ns_parse_simulation_mods_density_t *const density = mod->densities[i_d];
                    ns_increase_density(ns, density->x, density->y);
                }

                for (u_int64_t i_f = 0; i_f < mod->forces_length; ++i_f) {
                    const ns_parse_simulation_mods_force_t *const force = mod->forces[i_f];
                    ns_apply_force(ns, force->x, force->y, force->velocity.x, force->velocity.y);
                }
            }

            if (tick != 0) ns_tick(ns);

            for (size_t y = 0; y < world->world_height_bounds; ++y) {
                for (size_t x = 0; x < world->world_width_bounds; ++x) {
                    // Normalized Y used to invert the origin in bottom-left and not top-right
                    size_t nY = world->world_height_bounds - 1 - y;

                    int density = (int) (255 * *world->world[y][x].density);
                    if (density >= 255) density = 255;

                    // To print in output.txt correctly
                    fprintf(fp, "%ld, %ld, %d\n", x, nY, density);
                }
            }
        }

        fclose(fp);
        ns_free_world(world);
        ns_free(ns);
    }

    ns_parse_simulations_free(simulations);

    return EXIT_SUCCESS;
}
