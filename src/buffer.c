/**
 * @file buffer.c
 * @author Rob Griffith
 */

#include "buffer.h"

#include <string.h>

#include <stdio.h>

void buf_init(buffer_t *buf, size_t cap)
{
    buf->data = realloc(buf->data, cap);
    buf->capacity = buf->data ? cap : 0;
    buf->size = 0;
}

void buf_copy(buffer_t *buf, const void *src, size_t sz)
{
    buf_init(buf, sz);
    if (buf->data)
    {
        memcpy(buf->data, src, sz);
        buf->size = sz;
    }
}

void buf_move(buffer_t *buf, buffer_t *src)
{
    buf->data = src->data;
    buf->capacity = src->capacity;
    buf->size = src->size;
    /* clear source */
    memset(src, 0, sizeof(*src));
}

void buf_free(buffer_t *buf)
{
    if (buf)
    {
        free(buf->data);
    }
    memset(buf, 0, sizeof(*buf));
}

void buf_clear(buffer_t *buf)
{
    buf->size = 0;
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
    size_t reqsz = insz + sz;
    if (sz <= buf->capacity - buf->size)
    {
        /* simple copy */
        memcpy((char *)buf->data + insz, src, sz);
        buf->size = reqsz;
    }
    else
    {
        /* resize */
        const size_t maxsz = ~(size_t)0 - insz;
        sz = sz > maxsz ? maxsz : sz;
        reqsz = insz + sz;
        const size_t dcap = buf->capacity * 2;
        const size_t reqcap = dcap < reqsz ? reqsz : dcap;
        if (buf_resize_strict(buf, reqcap))
        {
            memcpy((char *)buf->data + insz, src, sz);
            buf->size = reqsz;
        }
        else
        {
            sz = 0;
        }
    }
    return sz;
}

size_t buf_push_strict(buffer_t *buf, const void *src, size_t sz)
{
    const size_t insz = buf->size;
    size_t reqsz = insz + sz;
    if (sz <= buf->capacity - buf->size)
    {
        /* simple copy */
        memcpy((char *)buf->data + insz, src, sz);
        buf->size = reqsz;
    }
    else
    {
        /* resize */
        const size_t maxsz = ~(size_t)0 - insz;
        sz = sz > maxsz ? maxsz : sz;
        reqsz = insz + sz;
        if (buf_resize_strict(buf, reqsz))
        {
            memcpy((char *)buf->data + insz, src, sz);
            buf->size = reqsz;
        }
        else
        {
            sz = 0;
        }
    }
    return sz;
}