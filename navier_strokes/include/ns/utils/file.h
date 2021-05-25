#ifndef _NS_UTILS_FILE_H
#define _NS_UTILS_FILE_H

#include <stdbool.h>

/**
 * Read the file at file_path.
 * The file must be textual.
 * Remember to free with free.
 *
 * @param file_path File location
 * @param error Error if something goes wrong, NULL otherwise
 * @return File content
 */
char *read_file(const char *file_path, char *error);

/**
 * Write content to file at file_path.
 * If the file does not exists, it will be create
 *
 * @param file_path File location
 * @param content File content
 * @param error Error if something goes wrong, NULL otherwise
 * @return true if write was successfully, false otherwise
 */
bool write_file(const char *file_path, const char *content, char *error);

#endif
