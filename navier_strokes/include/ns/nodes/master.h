#ifndef _NS_NODES_MASTER_H
#define _NS_NODES_MASTER_H

/**
 * Master nodes arguments.
 */
typedef struct comms_master_args_t {
    const char *simulations_file_path;
} comms_master_args_t;

/**
 * Execute master operations.
 *
 * @param args Master arguments
 */
void do_master(const comms_master_args_t *args);

#endif
