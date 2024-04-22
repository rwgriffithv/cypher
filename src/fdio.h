/**
 * @file fdio.h
 * @author Rob Griffith
 */

#ifndef FDIO_H
#define FDIO_H

#include "bufferedio.h"

/**
 * @brief initialize buffered I/O context to wrap a given file descriptor
 *
 * @ref bufferedio initialized to use read(2), write(2), lseek(2) and give appropriate status.
 * After wrapping, the buffered I/O context should be used with the buffered I/O API.
 *
 * @param[inout] fdb buffered I/O context to use
 * @param fd file descriptor
 * @param bufsz buffer size to use
 */
void fdio_wrap(bufferedio_t *fdb, int fd, size_t bufsz);

#endif