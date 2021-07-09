#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <argparse.h>
#include <dirent.h>
#include "ns/config.h"
#include "ns/nodes/master.h"
#include "ns/nodes/worker.h"
#include "ns/utils/logger.h"
#include "ns/utils/time_measurement.h"

static const char *description = "\n" PROJECT_DESCRIPTION "\n\tv." PROJECT_VERSION;
static const char *epilog = "\n© Carlo Corradini & Massimiliano Fronza";
static const char *const usage[] = {
        "mpiexec -np <#> ./navierstokes --simulations=./simulations.json --results=./results",
        "mpiexec -np <#> ./navierstokes --simulations=./simulations.json --results=./results --colors",
        "mpiexec -np <#> ./navierstokes --simulations=./simulations.json --results=./results --loglevel=DEBUG",
        "mpiexec -np <#> ./navierstokes --simulations=./simulations.json --results=./results --colors --loglevel=DEBUG",
        NULL
};

// Arguments
static struct {
    char *simulations;
    char *results;
    char *loglevel;
    bool colors;
} args = {
        .simulations = NULL,
        .results = NULL,
        .loglevel = "INFO",
        .colors = false,
};

static void make_args(int argc, const char **argv);

static bool check_args(void);

int main(int argc, const char **argv) {
    make_args(argc, argv);
    int rank;
    int size;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    log_set_rank(rank);
    log_set_level(log_level_int(args.loglevel));
    log_set_colors(args.colors);

    if (!check_args()) MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    if (size < 2) {
        log_error("At least two processes are required, %d given", size);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (rank == 0) {
        // Master
        time_measurement_t time;
        node_master_args_t master_args = {.simulations_path = args.simulations};

        time_measurement_start(&time);
        do_master(&master_args);
        time_measurement_stop_and_print(&time, "Master execution time");
    } else {
        // Worker
        time_measurement_t time;
        node_worker_args_t worker_args = {.results_path = args.results};

        time_measurement_start(&time);
        do_worker(&worker_args);
        time_measurement_stop_and_print(&time, "Worker execution time");
    }

    log_info("Terminating...");
    MPI_Finalize();

    return EXIT_SUCCESS;
}

static void make_args(int argc, const char **argv) {
    struct argparse argparse;
    struct argparse_option options[] = {
            OPT_GROUP("Options:"),
            OPT_HELP(),
            OPT_STRING(0, "simulations", &args.simulations, "Path to JSON simulations file", NULL, 0, OPT_NONEG),
            OPT_STRING(0, "results", &args.results, "Path to folder used to save JSON simulation results", NULL, 0,
                       OPT_NONEG),
            OPT_STRING(0, "loglevel", &args.loglevel, "Logger level. Default to `INFO`", NULL, 0, OPT_NONEG),
            OPT_BOOLEAN(0, "colors", &args.colors, "Enable logger output with colors", NULL, 0,
                        OPT_NONEG),
            OPT_END(),
    };

    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse, description, epilog);
    argparse_parse(&argparse, argc, argv);
}

static bool check_args(void) {
    // Simulations
    if (args.simulations == NULL) {
        log_error("`simulations` argument missing or invalid");
        return false;
    }
    // Results
    if (args.results == NULL) {
        log_error("`results` argument missing or invalid");
        return false;
    }
    if (args.results[strlen(args.results) - 1] == '/')
        args.results[strlen(args.results) - 1] = '\0';
    // Check if results directory exists
    DIR *results_dir = opendir(args.results);
    if (results_dir) closedir(results_dir);
    else {
        log_error("Results folder is invalid: %s", args.results);
        return false;
    }

    return true;
}
