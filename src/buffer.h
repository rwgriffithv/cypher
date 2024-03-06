/**
 * @file buffer.h
 * @author Rob Griffith
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>

typedef struct buffer
{
    void *data;
    size_t capacity;
    size_t size;
} buffer_t;

void buf_init(buffer_t *buf, size_t cap);

void buf_copy(buffer_t *buf, const void *src, size_t sz);

void buf_move(buffer_t *buf, buffer_t *src);

void buf_free(buffer_t *buf);

void buf_clear(buffer_t *buf);

size_t buf_resize(buffer_t *buf, size_t sz);

size_t buf_resize_strict(buffer_t *buf, size_t sz);

size_t buf_push(buffer_t *buf, const void *src, size_t sz);

size_t buf_push_strict(buffer_t *buf, const void *src, size_t sz);

#endif