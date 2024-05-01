/**
 * @file buffer.h
 * @author Rob Griffith
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>

/**
 * @struct buffer
 * @brief control over dynamically allocated region of memory
 * @typedef buffer_t
 */
typedef struct buffer
{
    void *data;      /** the allocated bytes */
    size_t capacity; /** the number of allocated bytes */
    size_t size;     /** the number of bytes considered used */
} buffer_t;

/**
 * @brief initialize buffer with specified number of bytes allocated
 *
 * @param[inout] buf the buffer to initialize
 * @param cap the number of bytes to allocate
 * @return the number of bytes allocated (cap if succesful, 0 if failed)
 */
size_t buf_init(buffer_t *buf, size_t cap);

/**
 * @brief copy data into buffer
 *
 * @param[inout] buf the buffer to copy into
 * @param src the data to copy
 * @param sz the number of bytes to copy
 * @return the number of bytes copied (sz if successful, 0 if failed)
 */
size_t buf_copy(buffer_t *buf, const void *src, size_t sz);

/**
 * @brief move data ownership from one buffer to another
 *
 * the source buffer will be cleared and set to all 0
 *
 * @param[inout] buf the buffer to own the data
 * @param[inout] src the buffer currently owning the data
 */
void buf_move(buffer_t *buf, buffer_t *src);

/**
 * @brief free the memory owned by the buffer
 *
 * the buffer will be cleared and set to all 0
 *
 * @param[inout] buf the buffer to free memory from
 */
void buf_free(buffer_t *buf);

/**
 * @brief clear buffer without freeing underlying allocated memory
 *
 * @param[inout] buf the buffer to clear (size will become 0)
 */
void buf_clear(buffer_t *buf);

/**
 * @brief set size of buffer, allocating memory as needed
 *
 * @param[inout] buf the buffer to resize
 * @param sz the
 * @return the new size of the buffer (sz if succesful)
 */
size_t buf_resize(buffer_t *buf, size_t sz);

/**
 * @brief push bytes to end of buffer, allocating memory as needed
 *
 * @param[inout] buf the buffer to push to
 * @param src the bytes to push
 * @param sz the number of bytes to push
 * @return the number of bytes pushed (sz if successful, 0 if failed)
 */
size_t buf_push(buffer_t *buf, const void *src, size_t sz);

#endif