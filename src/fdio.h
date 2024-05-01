/**
 * @file fdio.h
 * @author Rob Griffith
 */

#ifndef FDIO_H
#define FDIO_H

#include "bufferedio.h"

/**
 * @def FDIO_CLOSE
 * @brief file descriptor buffered I/O flag for invoking close(2) in cleanup
 */
#define FDIO_CLOSE 1

/**
 * @brief initialize buffered I/O context to wrap a given file descriptor
 *
 * @ref bufferedio initialized to use read(2), write(2), lseek(2) and give appropriate status.
 * After wrapping, the buffered I/O context should be used with the buffered I/O API.
 *
 * @param[inout] fdb buffered I/O context to use
 * @param fd file descriptor
 * @param bufsz buffer size to use
 * @param cflags flags for controlling manipulation of fd
 */
void fdio_wrap(bufferedio_t *fdb, int fd, size_t bufsz, int cflags);

#endif