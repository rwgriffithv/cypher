/**
 * @file buffer.c
 * @author Rob Griffith
 */

#include "buffer.h"

#include <string.h>

struct buffer
{
    void *data;
    size_t size;
    size_t capacity;
};

buffer_t *buf_init(size_t sz)
{
    const size_t cap = sz >= sizeof(size_t) ? sz : sizeof(size_t);
    buffer_t *buf = malloc(sizeof(buffer_t));
    void *data = calloc(1, cap);
    if (buf && data)
    {
        buf->data = data;
        buf->capacity = cap;
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

buffer_t *buf_copy(const void *src, size_t sz)
{
    buffer_t *buf = buf_init(sz);
    if (buf)
    {
        memcpy(buf->data, src, sz);
    }
    return buf;
}

buffer_t *buf_concat(const void *src_0, size_t sz_0, const void *src_1, size_t sz_1)
{
    buffer_t *buf = buf_init(sz_0 + sz_1);
    if (buf)
    {
        memcpy(buf->data, src_0, sz_0);
        memcpy(buf->data + sz_0, src_1, sz_1);
    }
    return buf;
}

void buf_free(buffer_t *buf)
{
    if (buf && buf->data)
    {
        free(buf->data);
    }
    free(buf);
}

size_t buf_resize(buffer_t *buf, size_t sz)
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

size_t buf_resize_strict(buffer_t *buf, size_t sz)
{
    buf->data = realloc(buf->data, sz);
    buf->capacity = buf->data ? sz : 0;
    buf->size = buf->capacity;
    return buf->size;
}

void *buf_data(buffer_t *buf)
{
    return buf->data;
}

const void *buf_cdata(const buffer_t *buf)
{
    return buf->data;
}

size_t buf_size(const buffer_t *buf)
{
    return buf->size;
}

size_t buf_capacity(const buffer_t *buf)
{
    return buf->capacity;
}

size_t buf_push(buffer_t *buf, const void *src, size_t sz)
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

size_t buf_push_strict(buffer_t *buf, const void *src, size_t sz)
{
    const size_t insz = buf->size;
    if (buf_resize(buf, buf->size + sz))
    {
        memcpy(((char *)buf->data) + insz, src, sz);
    }
    return buf->size;
}