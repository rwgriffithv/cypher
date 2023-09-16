/**
 * @file buffer.h
 * @author Rob Griffith
 */

#include <stdlib.h>

typedef struct buffer *buffer_h;

buffer_h buf_init(size_t sz);

buffer_h buf_copy(void *src, size_t sz);

buffer_h buf_concat(void *src_0, size_t sz_0, void *src_1, size_t sz_1);

void buf_free(buffer_h buf);

size_t buf_resize(buffer_h buf, size_t sz);

size_t buf_resize_strict(buffer_h buf, size_t sz);

void *buf_data(buffer_h buf);

size_t buf_size(buffer_h buf);

size_t buf_push(buffer_h buf, void *src, size_t sz);

size_t buf_push_strict(buffer_h buf, void *src, size_t sz);