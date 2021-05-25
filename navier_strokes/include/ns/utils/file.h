#ifndef _NS_UTILS_FILE_H
#define _NS_UTILS_FILE_H

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

#endif
