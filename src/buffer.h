/**
 * @file buffer.h
 * @author Rob Griffith
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>

typedef struct buffer buffer_t;

buffer_t *buf_init(size_t sz);

buffer_t *buf_copy(const void *src, size_t sz);

buffer_t *buf_concat(const void *src_0, size_t sz_0, const void *src_1, size_t sz_1);

void buf_free(buffer_t *buf);

size_t buf_resize(buffer_t *buf, size_t sz);

size_t buf_resize_strict(buffer_t *buf, size_t sz);

void *buf_data(buffer_t *buf);

const void *buf_cdata(const buffer_t *buf);

size_t buf_size(const buffer_t *buf);

size_t buf_capacity(const buffer_t *buf);

size_t buf_push(buffer_t *buf, const void *src, size_t sz);

size_t buf_push_strict(buffer_t *buf, const void *src, size_t sz);

#endif