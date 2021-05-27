#include "ns/nodes/com/message.h"

void com_message_MPI_datatype(MPI_Datatype *message_type) {
    if (message_type == NULL) return;

    // Number of items
    enum { n_items = 2 };

    // How many elements for each item
    int block_lengths[n_items] = {1, 1};

    // Type of each item
    MPI_Datatype types[n_items] = {MPI_C_BOOL, MPI_UINT64_T};

    // Calculate offsets
    MPI_Aint offsets[n_items];
    struct com_message_t m;
    MPI_Aint base_address;
    MPI_Get_address(&m, &base_address);
    MPI_Get_address(&m.terminate, &offsets[0]);
    MPI_Get_address(&m.simulation_id, &offsets[1]);
    offsets[0] = MPI_Aint_diff(offsets[0], base_address);
    offsets[1] = MPI_Aint_diff(offsets[1], base_address);

    // Create the struct type
    MPI_Type_create_struct(n_items, block_lengths, offsets, types, message_type);
    MPI_Type_commit(message_type);
}
