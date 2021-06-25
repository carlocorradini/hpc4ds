#include "ns/nodes/worker.h"
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <cJSON.h>
#include "ns/solver.h"
#include "ns/utils/logger.h"
#include "ns/utils/parser.h"
#include "ns/utils/file.h"
#include "ns/nodes/com/message.h"

#define MASTER_NODE_RANK 0
#define RESULT_FILE_MAX_NAME_LENGTH 64

static ns_parse_simulation_mod_t *find_mod_by_tick(const ns_simulation_t *simulation, uint64_t tick);

static bool write_simulation_metadata_to_result(cJSON *result_json, const ns_simulation_t *simulation);

void do_worker(const node_worker_args_t *const args) {
    int rank;
    int size;
    MPI_Datatype message_type;
    com_message_t message = {.terminate = false};
    MPI_Status status;
    char *simulation_string = NULL;
    int simulation_string_length;
    ns_simulation_t *simulation = NULL;
    ns_t *ns = NULL;
    ns_world_t *world = NULL;
    cJSON *result_json = NULL;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    com_message_MPI_datatype(&message_type);

    // Lifecycle
    log_info("Starting lifecycle");
    while (!message.terminate) {
        log_info("Listening...");

        // Wait a message from master
        MPI_Recv(&message, 1, message_type, MASTER_NODE_RANK, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        log_info("Received message");

        // If message is of type terminate, terminate lifecycle
        if (message.terminate) {
            log_info("TERMINATE");
            continue;
        }

        log_info("Simulation id: %ld", message.simulation_id);
        log_info("Waiting simulation...");
        // Obtain simulation length in chars
        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &simulation_string_length);

        // Allocate buffer just big enough to hold the incoming buffer
        simulation_string = (char *) calloc((unsigned long) simulation_string_length, sizeof(char));
        if (simulation_string == NULL) {
            log_error("Unable to allocate simulation string buffer of %d chars", simulation_string_length);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Read simulation data
        log_info("Reading simulation %ld composed by %d chars", message.simulation_id, simulation_string_length);
        MPI_Recv(simulation_string, simulation_string_length, MPI_CHAR, MASTER_NODE_RANK, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        // Parse simulation
        log_info("Parsing simulation %ld", message.simulation_id);
        simulation = ns_parse_simulation(simulation_string);
        if (simulation == NULL) {
            log_error("Unable to parse simulation %ld", message.simulation_id);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        free(simulation_string);

        // Create Navier Stokes simulation
        ns = ns_create(simulation->world.width, simulation->world.height,
                       simulation->fluid.viscosity, simulation->fluid.density, simulation->fluid.diffusion,
                       simulation->time_step);
        if (ns == NULL) {
            log_error("Unable to allocate ns structure");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Obtain Navier Stokes world snapshot
        world = ns_get_world(ns);
        if (world == NULL) {
            log_error("Unable to allocate world structure");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Populate simulation JSON with simulation data
        result_json = cJSON_CreateObject();
        if (cJSON_AddNumberToObject(result_json, "id", (double) message.simulation_id) == NULL
            || !write_simulation_metadata_to_result(result_json, simulation)) {
            log_error("Error adding metadata to JSON simulation");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Start simulation composed by ticks + 1 (world at tick 0)
        log_info("Starting simulation %ld composed by %ld ticks", message.simulation_id, simulation->ticks);
        cJSON *snapshots = cJSON_AddArrayToObject(result_json, "snapshots");
        if (snapshots == NULL) {
            log_error("Error adding snapshots to JSON simulation");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        for (uint64_t tick = 0; tick <= simulation->ticks; ++tick) {
            log_debug("Init tick %ld", tick);

            // Find a mod based on the current tick
            const ns_parse_simulation_mod_t *const mod = find_mod_by_tick(simulation, tick);

            if (mod != NULL) {
                // A mod has been found, apply it
                log_debug("Applying mod for tick %ld", tick);

                // Densities
                for (uint64_t i_d = 0; i_d < mod->densities_length; ++i_d) {
                    const ns_parse_simulation_mods_density_t *const density = mod->densities[i_d];
                    ns_increase_density(ns, density->x, density->y);
                }

                // Forces
                for (uint64_t i_f = 0; i_f < mod->forces_length; ++i_f) {
                    const ns_parse_simulation_mods_force_t *const force = mod->forces[i_f];
                    ns_apply_force(ns, force->x, force->y, force->velocity.x, force->velocity.y);
                }
            } else {
                // No mod has been found
                log_debug("Mod not found for tick %ld", tick);
            }

            // Compute a tick if this is not the first one.
            // This is done to obtain the initial world status.
            log_debug("Computing tick %ld", tick);
            if (tick != 0) ns_tick(ns);
            log_debug("Tick %ld computed", tick);

            // Compute world snapshot
            cJSON *snapshot = cJSON_CreateArray();
            if (snapshot == NULL) {
                log_error("Error creating JSON snapshot");
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }

            log_debug("Saving world snapshot on tick %ld", tick);
            for (size_t y = 0; y < world->world_height_bounds; ++y) {
                for (size_t x = 0; x < world->world_width_bounds; ++x) {
                    const ns_cell_t *const cell = &world->world[y][x];
                    cJSON *cell_json = NULL;

                    cell_json = cJSON_CreateObject();
                    if (cell_json == NULL) {
                        log_error("Error creating JSON cell");
                        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
                    }

                    if (cJSON_AddNumberToObject(cell_json, "x", (double) x) == NULL
                        || cJSON_AddNumberToObject(cell_json, "y", (double) y) == NULL
                        || cJSON_AddNumberToObject(cell_json, "d", *cell->density) == NULL
                        || cJSON_AddNumberToObject(cell_json, "u", *cell->u) == NULL
                        || cJSON_AddNumberToObject(cell_json, "v", *cell->v) == NULL) {
                        log_error("Unable to add data to JSON cell");
                        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
                    }

                    if (!cJSON_AddItemToArray(snapshot, cell_json)) {
                        log_error("Unable to add JSON cell to snapshot");
                        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
                    }
                }
            }

            if (!cJSON_AddItemToArray(snapshots, snapshot)) {
                log_error("Unable to add JSON snapshot to snapshots");
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }
        }
        log_info("Simulation ticks computed");

        log_info("Computing result data...");

        // Transform JSON object to string
        char *result_string = cJSON_Print(result_json);
        if (result_string == NULL) {
            log_error("Error transforming JSON result to string");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Save result to file
        size_t result_save_location_length = strlen(args->results_path) + 1 + RESULT_FILE_MAX_NAME_LENGTH + 1;
        char *result_save_location = (char *) calloc(result_save_location_length, sizeof(char));
        if (result_save_location == NULL) {
            log_error("Unable to allocate memory for save result location");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        snprintf(result_save_location, result_save_location_length, "%s/simulation_%ld_%d.json", args->results_path,
                 message.simulation_id, rank);

        log_info("Saving simulation %ld to file %s", message.simulation_id, result_save_location);
        char file_error[MPI_MAX_ERROR_STRING + 1];
        if (!write_file(result_save_location, result_string, file_error)) {
            log_error("Error saving file %s: %s", result_save_location, file_error);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        free(result_save_location);
        log_info("Save to file completed");

        log_info("Simulation %ld terminated", message.simulation_id);

        // Inform master that I can work again
        const com_message_t work_message = {.simulation_id = message.simulation_id, .terminate = false};
        log_debug("Sending work again message to master");
        MPI_Send(&work_message, 1, message_type, MASTER_NODE_RANK, 0, MPI_COMM_WORLD);
        log_debug("Message work again sent");

        cJSON_Delete(result_json);
        free(result_string);
        ns_parse_simulation_free(simulation);
        ns_free_world(world);
        ns_free(ns);
    }
    log_info("Lifecycle terminated");

    MPI_Type_free(&message_type);
}

static ns_parse_simulation_mod_t *find_mod_by_tick(const ns_simulation_t *const simulation, uint64_t tick) {
    if (simulation == NULL || simulation->mods == NULL || tick < 0 || tick > simulation->ticks)
        return NULL;

    for (uint64_t i = 0; i < simulation->mods_length; ++i) {
        ns_parse_simulation_mod_t *mod = simulation->mods[i];

        if (mod->tick == tick) return mod;
    }

    return NULL;
}

static bool write_simulation_metadata_to_result(cJSON *result_json, const ns_simulation_t *const simulation) {
    if (result_json == NULL || simulation == NULL) return false;
    cJSON *metadata_json = NULL;
    cJSON *world_json = NULL;
    cJSON *fluid_json = NULL;

    metadata_json = cJSON_AddObjectToObject(result_json, "metadata");
    if (metadata_json == NULL) return false;

    if (cJSON_AddNumberToObject(metadata_json, "time_step", simulation->time_step) == NULL
        || cJSON_AddNumberToObject(metadata_json, "ticks", (double) simulation->ticks) == NULL)
        return false;

    world_json = cJSON_AddObjectToObject(metadata_json, "world");
    if (world_json == NULL) return false;
    if (cJSON_AddNumberToObject(world_json, "width", (double) simulation->world.width) == NULL
        || cJSON_AddNumberToObject(world_json, "height", (double) simulation->world.height) == NULL
        || cJSON_AddNumberToObject(world_json, "width_bounds", (double) simulation->world.width + 2) == NULL
        || cJSON_AddNumberToObject(world_json, "height_bounds", (double) simulation->world.height + 2) == NULL)
        return false;

    fluid_json = cJSON_AddObjectToObject(metadata_json, "fluid");
    if (fluid_json == NULL) return false;
    if (cJSON_AddNumberToObject(fluid_json, "viscosity", simulation->fluid.viscosity) == NULL
        || cJSON_AddNumberToObject(fluid_json, "density", simulation->fluid.density) == NULL
        || cJSON_AddNumberToObject(fluid_json, "diffusion", simulation->fluid.diffusion) == NULL)
        return false;

    return true;
}
