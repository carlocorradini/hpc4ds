#include "ns/nodes/worker.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "ns/utils/parser.h"

static ns_parse_simulation_mod_t *find_mod_by_tick(const ns_simulation_t *simulation, uint64_t tick);

void do_worker(void) {
    int rank;
    int size;
    MPI_Status status;
    int chars;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_CHAR, &chars);

    // Allocate a buffer just big enough to hold the incoming buffer
    char *simulation_string = (char *) calloc((unsigned long) chars, sizeof(char));
    MPI_Recv(simulation_string, chars, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("Worker %d Reading: %s\n", rank, simulation_string);

    ns_simulation_t *simulation = ns_parse_simulation(simulation_string);
    free(simulation_string);
    ns_parse_simulation_free(simulation);

    /*ns_simulation_t *simulation = NULL;
        ns_t *ns = NULL;
        ns_world_t *world = NULL;

        simulation = simulations->simulations[i_s];
        ns = ns_create(simulation->world.width, simulation->world.height,
                       simulation->fluid.viscosity, simulation->fluid.density, simulation->fluid.diffusion,
                       simulation->time_step);
        world = ns_get_world(ns);

        for (uint64_t tick = 0; tick < simulation->ticks; ++tick) {
            const ns_parse_simulation_mod_t *const mod = find_mod_by_tick(simulation, tick);

            if (mod != NULL) {
                for (uint64_t i_d = 0; i_d < mod->densities_length; ++i_d) {
                    const ns_parse_simulation_mods_density_t *const density = mod->densities[i_d];
                    ns_increase_density(ns, density->x, density->y);
                }

                for (uint64_t i_f = 0; i_f < mod->forces_length; ++i_f) {
                    const ns_parse_simulation_mods_force_t *const force = mod->forces[i_f];
                    ns_apply_force(ns, force->x, force->y, force->velocity.x, force->velocity.y);
                }
            }

            if (tick != 0) ns_tick(ns);

            for (size_t y = 0; y < world->world_height_bounds; ++y) {
                for (size_t x = 0; x < world->world_width_bounds; ++x) {
                    const double density = *world->world[y][x].density;
                }
            }
        }

        ns_free_world(world);
        ns_free(ns);*/
}

static ns_parse_simulation_mod_t *find_mod_by_tick(const ns_simulation_t *const simulation, uint64_t tick) {
    if (simulation == NULL || simulation->mods == NULL || tick < 0 || tick > simulation->ticks - 1)
        return NULL;

    for (uint64_t i = 0; i < simulation->mods_length; ++i) {
        ns_parse_simulation_mod_t *mod = simulation->mods[i];

        if (mod->tick == tick) return mod;
    }

    return NULL;
}