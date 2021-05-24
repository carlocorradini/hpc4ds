#include <stdlib.h>
#include <mpi.h>
#include <argparse.h>
#include "ns/config.h"
#include "ns/nodes/master.h"
#include "ns/nodes/worker.h"
#include "ns/utils/logger.h"

static const char *description = PROJECT_DESCRIPTION "\n\tv." PROJECT_VERSION;
static const char *epilog = "\n© Carlo Corradini & Massimiliano Fronza";
static const char *const usage[] = {
        "mpirun -n <#> ./navierstokes --simulations=./simulations.json",
        "mpirun -n <#> ./navierstokes --simulations=./simulations.json --colors",
        "mpirun -n <#> ./navierstokes --simulations=./simulations.json --colors --loglevel=DEBUG",
        NULL
};

// Arguments
static struct {
    char *simulations;
    char *loglevel;
    bool colors;
} args = {
        .simulations = NULL,
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
        node_master_args_t master_args = {.simulations_file_path = args.simulations};
        do_master(&master_args);
    } else {
        // Worker
        node_worker_args_t worker_args = {};
        do_worker(&worker_args);
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
            OPT_STRING(0, "loglevel", &args.loglevel, "Logger minimum level. Default to `INFO`", NULL, 0, OPT_NONEG),
            OPT_BOOLEAN(0, "colors", &args.colors, "Enable logger output with colors.", NULL, 0,
                        OPT_NONEG),
            OPT_END(),
    };

    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse, description, epilog);
    argparse_parse(&argparse, argc, argv);
}

static bool check_args(void) {
    if (args.simulations == NULL) {
        log_error("`simulations` argument missing or invalid");
        return false;
    }

    return true;
}
