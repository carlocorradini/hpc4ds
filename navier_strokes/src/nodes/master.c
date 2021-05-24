#include "ns/nodes/master.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <mpi.h>
#include "ns/utils/logger.h"
#include "ns/utils/parser.h"
#include "ns/utils/stringify.h"
#include "ns/nodes/com/message.h"

void do_master(const node_master_args_t *const args) {
    int rank;
    int size;
    ns_simulations_t *simulations = NULL;
    MPI_Datatype message_type;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    com_message_MPI_datatype(&message_type);

    // TODO remove
    const char *string = "{\"simulations\":[{\"time_step\":0.01,\"ticks\":50,\"world\":{\"width\":100,\"height\":100},\"fluid\":{\"viscosity\":0.0001,\"density\":10,\"diffusion\":0.0001},\"mods\":[{\"tick\":0,\"densities\":[{\"x\":41,\"y\":41},{\"x\":65,\"y\":20},{\"x\":15,\"y\":20}],\"forces\":[{\"x\":41,\"y\":41,\"velocity\":{\"x\":0,\"y\":80}}]}]}]}";

    log_info("Parsing simulations file %s", args->simulations_file_path);
    simulations = ns_parse_simulations(string);
    if (simulations == NULL) {
        log_error("Unable to parse simulations file `%s`", args->simulations_file_path);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    log_info("Processing %ld simulation%s", simulations->simulations_length,
             simulations->simulations_length > 1 ? "s" : "");
    for (uint64_t i_s = 0; i_s < simulations->simulations_length; ++i_s) {
        const ns_simulation_t *simulation = NULL;
        char *simulation_string = NULL;
        const int worker = size - 1;

        simulation = simulations->simulations[i_s];
        simulation_string = ns_stringify_simulation(simulation);
        if (simulation_string == NULL) {
            log_error("Unable to stringify simulation %ld", i_s);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        const com_message_t message = {.terminate = false, .simulation_id = i_s};

        log_info("Sending simulation metadata %ld to worker node %d", i_s, worker);
        MPI_Send(&message, 1, message_type, worker, 0, MPI_COMM_WORLD);
        log_info("Simulation metadata %ld sent", i_s);

        log_info("Sending simulation %ld to worker node %d", i_s, worker);
        MPI_Send(simulation_string, (int) strlen(simulation_string) + 1, MPI_CHAR, worker, 0, MPI_COMM_WORLD);
        log_info("Simulation %ld sent", i_s);

        free(simulation_string);
    }

    log_info("Sending termination message to all workers");
    for (int worker = 1; worker < size; ++worker) {
        const com_message_t message = {.terminate = true};

        log_info("Sending termination message to worker node %d", worker);
        MPI_Send(&message, 1, message_type, worker, 0, MPI_COMM_WORLD);
        log_info("Termination message sent");
    }
    log_info("Termination messages sent");

    MPI_Type_free(&message_type);
    ns_parse_simulations_free(simulations);
}
