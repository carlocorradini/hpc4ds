#include "comms_master.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>
#include "../ns/ns_parser.h"
#include "../ns/ns_stringify.h"
#include "../ns/ns_solver.h"

static ns_parse_simulation_mod_t *find_mod_by_tick(const ns_parse_simulation_t *simulation, uint64_t tick);

void do_master(const comms_master_args_t *const args) {
    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char *string = "{\"simulations\":[{\"time_step\":0.01,\"ticks\":50,\"world\":{\"width\":100,\"height\":100},\"fluid\":{\"viscosity\":0.0001,\"density\":10,\"diffusion\":0.0001},\"mods\":[{\"tick\":0,\"densities\":[{\"x\":41,\"y\":41},{\"x\":65,\"y\":20},{\"x\":15,\"y\":20}],\"forces\":[{\"x\":41,\"y\":41,\"velocity\":{\"x\":0,\"y\":80}}]}]}]}";

    ns_parse_simulations_t *simulations = ns_parse_simulations(string);

    for (uint64_t i_s = 0; i_s < simulations->simulations_length; ++i_s) {
        const ns_parse_simulation_t *simulation = NULL;
        char *simulation_string = NULL;
        int worker;

        simulation = simulations->simulations[i_s];
        simulation_string = ns_stringify_simulation(simulation);
        // FIXME
        worker = size - 1;

        fprintf(stdout, "Sending simulation %ld to worker node %d\n", i_s, worker);
        MPI_Send(simulation_string, (int) strlen(simulation_string), MPI_CHAR, worker, 0, MPI_COMM_WORLD);

        free(simulation_string);
    }

    ns_parse_simulations_free(simulations);
}

static ns_parse_simulation_mod_t *find_mod_by_tick(const ns_parse_simulation_t *const simulation, uint64_t tick) {
    if (simulation == NULL || simulation->mods == NULL || tick < 0 || tick > simulation->ticks - 1)
        return NULL;

    for (uint64_t i = 0; i < simulation->mods_length; ++i) {
        ns_parse_simulation_mod_t *mod = simulation->mods[i];

        if (mod->tick == tick) return mod;
    }

    return NULL;
}
