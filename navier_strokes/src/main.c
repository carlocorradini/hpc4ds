#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>
#include <argparse.h>
#include "comms/comms_master.h"
#include "comms/comms_slave.h"

static const char *description = "\nNavier Stokes simulations in high performance computing environment.";
static const char *epilog = "\n© Carlo Corradini & Massimiliano Fronza";
static const char *const usage[] = {
        "mpirun -n <#> ./navierstokes --simulations=./simulations.json",
        NULL
};

int main(int argc, const char **argv) {
    // Arguments
    struct argparse argparse;
    const char *arg_simulations = NULL;
    bool arg_m_worker = false;
    struct argparse_option options[] = {
            OPT_GROUP("Options:"),
            OPT_HELP(),
            OPT_STRING(0, "simulations", &arg_simulations, "Path to JSON simulations file", NULL, 0, OPT_NONEG),
            OPT_END(),
    };

    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse, description, epilog);
    argparse_parse(&argparse, argc, argv);

    if (arg_simulations == NULL) {
        fprintf(stderr, "[ERROR]: `simulations` argument missing or invalid\n");
        return EXIT_FAILURE;
    }
    // END Arguments

    int rank;
    int size;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        fprintf(stderr, "[ERROR]: At least two processes are required, %d given\n", size);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (rank == 0) {
        // Master
        comms_master_args_t master_args = {.simulations_file_path = arg_simulations};
        do_master(&master_args);
    } else {
        // Slave
        do_slave();
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
