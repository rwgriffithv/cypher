/**
 * @file buffer.c
 * @author Rob Griffith
 */

#include "buffer.h"

#include <stdio.h>
#include <string.h>

size_t _align_cap(size_t cap)
{
    /* used in internal reallocs, aligns data with words and prevents undesired free */
    return cap - (cap % sizeof(size_t)) + sizeof(size_t);
}

size_t buf_init(buffer_t *buf, size_t cap)
{
    buf->data = realloc(buf->data, cap);
    buf->capacity = buf->data ? cap : 0;
    buf->size = 0;
    return buf->capacity;
}

size_t buf_copy(buffer_t *buf, const void *src, size_t sz)
{
    if (buf_init(buf, _align_cap(sz)))
    {
        memcpy(buf->data, src, sz);
        buf->size = sz;
    }
    return buf->size;
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
        memset(buf, 0, sizeof(*buf));
    }
}

void buf_clear(buffer_t *buf)
{
    buf->size = 0;
}

size_t buf_resize(buffer_t *buf, size_t sz)
{
    if (sz <= buf->capacity || buf_init(buf, _align_cap(sz)))
    {
        buf->size = sz;
    }
    return buf->size;
}

size_t buf_push(buffer_t *buf, const void *src, size_t sz)
{
    const size_t insz = buf->size;
    if (sz > buf->capacity - insz)
    {
        /* resize, handle overflow */
        const size_t maxcap = ~(size_t)0;
        size_t outsz = insz + sz;
        outsz = outsz < insz ? maxcap : outsz;
        size_t dcap = buf->capacity * 2;
        dcap = dcap < buf->capacity ? maxcap : dcap;
        if (!buf_init(buf, _align_cap(dcap < outsz ? outsz : dcap)))
        {
            return 0;
        }
        sz = outsz - insz;
    }
    buf->size = insz + sz;
    if (src)
    {
        memcpy((char *)buf->data + insz, src, sz);
    }
    else
    {
        /* zero */
        memset((char *)buf->data + insz, 0, sz);
    }
    return sz;
}