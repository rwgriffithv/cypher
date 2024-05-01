/**
 * @file bufferedio.h
 * @author Rob Griffith
 */

#ifndef BUFFEREDIO_H
#define BUFFEREDIO_H

#include "buffer.h"

#include <sys/types.h>

/**
 * @def BIO_STATUS_INIT
 * @brief bufferedio status integer generality
 */
#define BIO_STATUS_INIT 0

/**
 * @struct bio_data
 * @brief data manipulated by buffered I/O API
 * @typedef bio_data_t
 */
typedef struct bio_data
{
    buffer_t buf;    /** buffer used to store bytes for I/O */
    size_t offset;   /** offset into buffered bytes */
    buffer_t opaque; /** buffer to store arbitrary data in */
} bio_data_t;

/**
 * @struct bufferedio
 * @brief context (data and functions) used with buffered I/O API
 * @typedef bufferedio_t
 *
 * Different types of I/O are supported by initializing the data and function pointers appropriately.
 * Generally, bufferedio structs should be zero-initialized.
 */
typedef struct bufferedio
{
    /** data containing buffered I/O context */
    bio_data_t data;
    /** status function pointer invoked by @ref bio_status */
    int (*status)(const bio_data_t *);
    /** string status function pointer invoked by @ref bio_status_str */
    void (*status_str)(const bio_data_t *, char *, size_t);
    /** read function pointer invoked by @ref bio_read */
    size_t (*read)(bio_data_t *, void *, size_t);
    /** write function pointer invoked by @ref bio_write */
    size_t (*write)(bio_data_t *, const void *, size_t);
    /** flush function pointer invoked by @ref bio_flush */
    void (*flush)(bio_data_t *);
    /** seek function pointer invoked by @ref bio_seek */
    ssize_t (*seek)(bio_data_t *, long, int);
    /** data free function pointer invoked by @ref bio_dfree */
    void (*dfree)(bio_data_t *);
} bufferedio_t;

/**
 * @brief wrap direct access to a memory buffer using the buffered I/O API
 *
 * Any buffer wrapped in a buffered I/O context is moved (owned) by the context using @ref buf_move
 * Writing will push to the buffer.
 * Reading retrieves bytes in a FIFO order.
 * Flushing clears the buffer.
 * Seeking will change where reads start from.
 *
 * @param[inout] bio buffered I/O context
 * @param[inout] buf the buffer to wrap (NULL to allocate a new buffer)
 */
void bio_wrap(bufferedio_t *bio, buffer_t *buf);

/**
 * @brief get status integer of buffered I/O context
 *
 * Invokes @ref bufferedio::status (must be set).
 * Status < @ref BIO_STATUS_INIT indicates I/O error.
 * Status > @ref BIO_STATUS_INIT indicates usability and expected operation.
 *
 * @param[inout] bio buffered I/O context
 * @return status integer of buffered I/O context
 */
int bio_status(const bufferedio_t *bio);

/**
 * @brief get status string of buffered I/O context
 *
 * Invokes @ref bufferedio::status_str if available.
 * Will set default string if no implementation provided in buffered I/O context.
 *
 * @param[inout] bio buffered I/O context
 * @param[out] str buffer to populate with status string
 * @param n number of bytes available in buffer to populate
 * @return pointer to string buffer input str
 */
char *bio_status_str(bufferedio_t *bio, char *str, size_t n);

/**
 * @brief read bytes using buffered I/O context
 *
 * Invokes @ref bufferedio::read (must be set) until sz bytes are read or EOF or error occurs.
 * @ref bufferedio::read is invoked repeatedly as needed.
 * Looping is handled here, so implementations of bufferedio::read should only attempt 1 read.
 * Less than requested bytes is returned on error or EOF, use @ref bio_status to check for error.
 *
 * @param[inout] bio buffered I/O context
 * @param[out] data output buffer to populate with bytes
 * @param sz number of bytes to read into output data buffer
 * @return number of bytes read into output data buffer
 */
size_t bio_read(bufferedio_t *bio, void *data, size_t sz);

/**
 * @brief read all available bytes from current position using buffered I/O context (until EOF or failure)
 *
 * If @ref bufferedio::seek using SEEK_END is supported, will invoke @ref bufferedio::read once.
 * If FIFO input or seeking to the end is not supported, will repeatedly invoke @ref bufferedio::read while pushing to buffer.
 * Returns NULL upon error, @ref bio_status and @ref bio_status_str provide I/O error insight.
 * @ref buffer::size provides memory allocation error insight.
 *
 * @param[inout] bio buffered I/O context
 * @param[inout] buf the buffer to populate
 * @return populated buffer (NULL if error)
 */
const buffer_t *bio_read_all(bufferedio_t *bio, buffer_t *buf);

/**
 * @brief write bytes using buffered I/O context
 *
 * Invokes @ref bufferedio::write (must be set) until sz bytes are written or error occurs.
 * @ref bufferedio::write is invoked repeatedly as needed.
 * Looping is handled here, so implementations of bufferedio::write should only attempt 1 write.
 * Less than requested bytes is returned on error or EOF, use @ref bio_status to check for error.
 *
 * @param[inout] bio buffered I/O context
 * @param[in] data input buffer of bytes to write
 * @param sz number of bytes to write from input data buffer
 * @return number of bytes written from input data buffer
 */
size_t bio_write(bufferedio_t *bio, const void *data, size_t sz);

/**
 * @brief flush buffered data in buffered I/O context
 *
 * Invokes @ref bufferedio::flush (must be set) once.
 * This should clear any buffered data by writing.
 * Generally flush should not be invoked when reading.
 *
 * @param[inout] bio buffered I/O context
 */
void bio_flush(bufferedio_t *bio);

/**
 * @brief seek into new byte position in buffered I/O context
 *
 * Invokes @ref bufferedio::sek (mus be set) once.
 * This will change the position at which bytes are read or written to next.
 *
 * @param[inout] bio buffered I/O context
 * @param offset the byte posiion as specified by whence (same as lseek(2))
 * @param whence the flag specifying how to apply offset (same as lseek(2))
 * @return the resulting byte position from beginning of file, -1 on error (same as lseek(2))
 */
ssize_t bio_seek(bufferedio_t *bio, long offset, int whence);

/**
 * @brief free the data portion of the buffered I/O context
 *
 * Invokes @ref bufferedio::dfree if available to allow for custom cleanup.
 * If @ref bufferedio_t::dfree is NULL, frees all buffers.
 *
 * @param[inout] bio buffered I/O context
 */
void bio_dfree(bufferedio_t *bio);

#endif