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
 * Use @ref bio_status to check for sucessful initialization.
 *
 * @param[inout] bio buffered I/O context to use
 * @param fd file descriptor
 * @param bufsz buffer size to use
 * @param cflags flags for controlling manipulation of fd
 */
void fdio_wrap(bufferedio_t *bio, int fd, size_t bufsz, int cflags);

#endif