/**
 * @file fileio.h
 * @author Rob Griffith
 */

#ifndef FILEIO_H
#define FILEIO_H

#include "bufferedio.h"

/**
 * @brief read all bytes from file into buffer
 *
 * Upon error(s), messages are printed to stderr.
 * Buffer size should be checked to determine number of bytes read.
 *
 * @param[inout] buf buffer to read bytes into
 * @param path path of file to read
 * @return < 0 upon error
 */
int fio_read_all(buffer_t *buf, const char *path);

/**
 * @brief write all bytes from buffer to file
 *
 * Upon error(s), messages are printed to stderr.
 *
 * @param path path of file to write
 * @param buf buffer to write bytes from
 * @return number of bytes written to file
 */
size_t fio_write_all(const char *path, const buffer_t *buf);

#endif