/**
 * @file buffer.c
 * @author Rob Griffith
 */

#include "buffer.h"

#include <string.h>

typedef struct buffer
{
    void *data;
    size_t size;
    size_t capacity;
} buffer_t;

buffer_h buf_init(size_t sz)
{
    buffer_h buf = malloc(sizeof(buffer_t));
    void *data = calloc(sz, sizeof(char));
    if (buf && data)
    {
        buf->data = data;
        buf->capacity = sz;
        buf->size = sz;
    }
    else
    {
        free(buf);
        free(data);
        buf = NULL;
    }
    return buf;
}

buffer_h buf_copy(const void *src, size_t sz)
{
    buffer_h buf = buf_init(sz);
    if (buf)
    {
        memcpy(buf->data, src, sz);
    }
    return buf;
}

buffer_h buf_concat(const void *src_0, size_t sz_0, const void *src_1, size_t sz_1)
{
    buffer_h buf = buf_init(sz_0 + sz_1);
    if (buf)
    {
        memcpy(buf->data, src_0, sz_0);
        memcpy(buf->data + sz_0, src_1, sz_1);
    }
    return buf;
}

void buf_free(buffer_h buf)
{
    if (buf && buf->data)
    {
        free(buf->data);
    }
    free(buf);
}

size_t buf_resize(buffer_h buf, size_t sz)
{
    if (sz <= buf->capacity)
    {
        buf->size = sz;
    }
    else
    {
        buf_resize_strict(buf, sz);
    }
    return buf->size;
}

size_t buf_resize_strict(buffer_h buf, size_t sz)
{
    buf->data = realloc(buf->data, sz);
    buf->capacity = buf->data ? sz : 0;
    buf->size = buf->capacity;
    return buf->size;
}

void *buf_data(buffer_h buf)
{
    return buf->data;
}

size_t buf_size(buffer_h buf)
{
    return buf ? buf->size : 0;
}

size_t buf_push(buffer_h buf, const void *src, size_t sz)
{
    const size_t insz = buf->size;
    const size_t reqsz = insz + sz;
    while (buf->capacity < reqsz)
    {
        /* double capacity until requirement met */
        buf->capacity = buf->capacity << 1;
    }
    if (buf_resize(buf, buf->capacity))
    {
        memcpy(((char *)buf->data) + insz, src, sz);
        buf->size = reqsz;
    }
    return buf->size;
}

size_t buf_push_strict(buffer_h buf, const void *src, size_t sz)
{
    const size_t insz = buf->size;
    if (buf_resize(buf, buf->size + sz))
    {
        memcpy(((char *)buf->data) + insz, src, sz);
    }
    return buf->size;
}