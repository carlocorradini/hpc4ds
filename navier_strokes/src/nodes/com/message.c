#include "ns/nodes/com/message.h"

void com_message_MPI_datatype(MPI_Datatype *message_type) {
    if (message_type == NULL) return;

    enum { n_items = 2 };
    int block_lengths[n_items] = {1, 1};
    MPI_Datatype types[n_items] = {MPI_C_BOOL, MPI_UINT64_T};
    MPI_Aint offsets[n_items] = {
            offsetof(com_message_t, terminate),
            offsetof(com_message_t, simulation_id)
    };

    MPI_Type_create_struct(n_items, block_lengths, offsets, types, message_type);
    MPI_Type_commit(message_type);
}