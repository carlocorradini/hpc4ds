#ifndef _NS_NODES_MASTER_H
#define _NS_NODES_MASTER_H

/**
 * Master node arguments.
 */
typedef struct node_master_args_t {
    const char *simulations;
} node_master_args_t;

/**
 * Execute master operations.
 *
 * @param args Master arguments
 */
void do_master(const node_master_args_t *args);

#endif
