#ifndef _NS_NODES_WORKER_H
#define _NS_NODES_WORKER_H

/**
 * Worker node arguments.
 */
typedef struct node_worker_args_t {
} node_worker_args_t;

/**
 * Execute worker operations.
 *
 * @param args Worker arguments
 */
void do_worker(const node_worker_args_t *args);

#endif
