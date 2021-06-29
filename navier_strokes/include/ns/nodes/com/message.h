#ifndef _NS_NODES_COM_MESSAGE_H
#define _NS_NODES_COM_MESSAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <mpi.h>

/**
 * Message type.
 */
typedef struct com_message_t {
    bool terminate;
    uint64_t simulation_id;
} com_message_t;

/**
 * Define MPI message datatype.
 *
 * @param message_type MPI message datatype
 */
void com_message_MPI_datatype(MPI_Datatype *message_type);

#endif
