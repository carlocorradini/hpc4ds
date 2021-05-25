#include "ns/utils/file.h"
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

char *read_file(const char *const file_path, char *error) {
    MPI_File fh;
    MPI_Offset filesize;
    size_t num_chars;
    char *buffer;
    int error_code;
    int error_length;

    // Open file at file_path
    error_code = MPI_File_open(MPI_COMM_SELF, file_path, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    if (error_code != MPI_SUCCESS) {
        MPI_Error_string(error_code, error, &error_length);
        return NULL;
    }

    // Get file size in byte
    MPI_File_get_size(fh, &filesize);
    // Obtain the number of chars
    num_chars = (size_t) filesize / sizeof(char);

    // Allocate string buffer
    buffer = (char *) calloc(num_chars, sizeof(char));
    if (buffer == NULL) {
        error = "Unable to allocate simulations file buffer";
        return NULL;
    }

    // Read the file and copy the content to buffer
    error_code = MPI_File_read(fh, buffer, (int) num_chars, MPI_CHAR, MPI_STATUS_IGNORE);
    if (error_code != MPI_SUCCESS) {
        MPI_Error_string(error_code, error, &error_length);
        return NULL;
    }

    MPI_File_close(&fh);
    return buffer;
}

bool write_file(const char *const file_path, const char *const content, char *error) {
    MPI_File fh;
    int error_code;
    int error_length;

    // Open file at file_path
    error_code = MPI_File_open(MPI_COMM_SELF, file_path, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
    if (error_code != MPI_SUCCESS) {
        MPI_Error_string(error_code, error, &error_length);
        return false;
    }

    // Write to file
    error_code = MPI_File_write(fh, content, (int) strlen(content), MPI_CHAR, MPI_STATUS_IGNORE);
    if (error_code != MPI_SUCCESS) {
        MPI_Error_string(error_code, error, &error_length);
        return false;
    }

    MPI_File_close(&fh);
    return true;
}
