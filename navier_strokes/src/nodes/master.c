#include "ns/nodes/master.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <mpi.h>
#include "ns/utils/logger.h"
#include "ns/utils/parser.h"
#include "ns/utils/stringify.h"
#include "ns/utils/file.h"
#include "ns/nodes/com/message.h"

typedef struct worker_t {
    int rank;
    bool working;
} worker_t;

void do_master(const node_master_args_t *const args) {
    int rank;
    int size;
    MPI_Datatype message_type;
    worker_t *workers = NULL;
    uint available_workers;
    ns_simulations_t *simulations = NULL;
    char *simulations_string = NULL;
    char file_error[MPI_MAX_ERROR_STRING + 1];

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    com_message_MPI_datatype(&message_type);
    available_workers = (uint) (size - 1);

    log_info("Workers available: %d", available_workers);

    // Read simulations file
    log_info("Reading simulations file at %s", args->simulations_path);
    simulations_string = read_file(args->simulations_path, file_error);
    if (simulations_string == NULL) {
        log_error("Error opening and managing simulations file: %s", file_error);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Parse simulation
    log_info("Parsing simulations file content");
    simulations = ns_parse_simulations(simulations_string);
    if (simulations == NULL) {
        log_error("Unable to parse simulations file `%s`", args->simulations_path);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    free(simulations_string);

    // Workers
    log_debug("Allocating workers");
    workers = (worker_t *) calloc(available_workers, sizeof(worker_t));
    if (workers == NULL) {
        log_error("Unable to allocate workers");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    for (uint worker = 0; worker < available_workers; ++worker) {
        workers[worker].rank = (int) worker + 1;
        workers[worker].working = false;
    }
    log_debug("Workers successfully initialized");

    // Show a warning message if the number of workers is more than the number of simulations
    if (available_workers > simulations->simulations_length)
        log_warn("%d workers available for only %ld simulation%s", available_workers, simulations->simulations_length,
                 simulations->simulations_length > 1 ? "s" : "");

    log_info("Processing %ld simulation%s", simulations->simulations_length,
             simulations->simulations_length > 1 ? "s" : "");
    for (uint64_t i_s = 0; i_s < simulations->simulations_length; ++i_s) {
        worker_t *worker;
        const ns_simulation_t *simulation = NULL;
        char *simulation_string = NULL;
        const com_message_t master_message = {.terminate = false, .simulation_id = i_s};

        // Obtain simulation and stringify it
        simulation = simulations->simulations[i_s];
        simulation_string = ns_stringify_simulation(simulation);
        if (simulation_string == NULL) {
            log_error("Unable to stringify simulation %ld", master_message.simulation_id);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Obtain a worker
        if (i_s < available_workers) {
            // All workers can work
            worker = &workers[i_s % available_workers];
        } else {
            // All workers are working. Wait for one worker to finish
            com_message_t worker_message;
            MPI_Status worker_status;

            log_info("Waiting a free worker...");
            MPI_Recv(&worker_message, 1, message_type, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &worker_status);
            log_info("Worker %ld has successfully completed simulation %ld", worker_status.MPI_SOURCE,
                     worker_message.simulation_id);
            log_info("Worker %ld can work", worker_status.MPI_SOURCE);

            worker = &workers[worker_status.MPI_SOURCE - 1];
            // Worker is not working
            worker->working = false;
        }

        // Check if something is wrong in the logic
        if (worker->working) {
            log_error("Sending a simulation to an already working node");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Set worker to working to prevent undefined behaviour
        worker->working = true;

        // Send simulation metadata
        log_info("Sending simulation metadata %ld to worker node %d", master_message.simulation_id, worker->rank);
        MPI_Send(&master_message, 1, message_type, worker->rank, 0, MPI_COMM_WORLD);
        log_info("Simulation metadata %ld sent", master_message.simulation_id);

        // Send simulation
        log_info("Sending simulation %ld to worker node %d", master_message.simulation_id, worker->rank);
        MPI_Send(simulation_string, (int) strlen(simulation_string) + 1, MPI_CHAR, worker->rank, 0, MPI_COMM_WORLD);
        log_info("Simulation %ld sent", master_message.simulation_id);

        free(simulation_string);
    }
    log_info("All simulations processed successfully");

    // Send termination messages
    log_info("Sending termination message to all workers");
    for (uint i = 0; i < available_workers; ++i) {
        worker_t *worker = &workers[i];
        const com_message_t message = {.terminate = true};

        // Check if worker is working
        if (worker->working) {
            // Working message not received
            com_message_t worker_message;

            // Wait message from worker
a            log_info("Worker %ld is working", worker->rank);
            log_info("Waiting worker %ld availability message...", worker->rank);
            MPI_Recv(&worker_message, 1, message_type, worker->rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            log_info("Worker %ld message received", worker->rank);

            // Worker is not working
            worker->working = false;
        }

        // Send termination message
        log_info("Sending termination message to worker node %d", worker->rank);
        MPI_Send(&message, 1, message_type, worker->rank, 0, MPI_COMM_WORLD);
        log_info("Termination message sent");
    }
    log_info("Termination messages sent");

    free(workers);
    ns_parse_simulations_free(simulations);
    MPI_Type_free(&message_type);
}
