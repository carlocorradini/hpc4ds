#include "comms_master.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>
#include "../ns/ns_parser.h"
#include "../ns/ns_stringify.h"

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
