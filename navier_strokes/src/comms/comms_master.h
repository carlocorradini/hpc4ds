#ifndef _COMMS_MASTER_H
#define _COMMS_MASTER_H

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
