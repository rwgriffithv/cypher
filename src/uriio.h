/**
 * @file uriio.h
 * @author Rob Griffith
 */

#ifndef URIIO_H
#define URIIO_H

#include "bufferedio.h"

/**
 * @brief initialize buffered I/O context using a given URI
 *
 * Use @ref bio_status to check for sucessful initialization.
 *
 * @param[inout] bio buffered I/O context to use
 * @param uri encoded URI specifying I/O source/destination
 * @param bufsz buffer size to use
 */
void uriio_init(bufferedio_t *bio, const char *uri);

#endif