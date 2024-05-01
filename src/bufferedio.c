/**
 * @file bufferedio.c
 * @author Rob Griffith
 */

#include "bufferedio.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

int _bio_wstatus(const bio_data_t *bd)
{
    /* always usable */
    return BIO_STATUS_INIT + 1;
}

void _bio_wstatus_str(const bio_data_t *bd, char *str, size_t n)
{
    const char *fmt = "wrapped buffer %sallocated {data: %zu, size: %zu, capacity: %zu}";
    const buffer_t *buf = &bd->buf;
    if (snprintf(str, n, fmt, buf->data ? "" : "not ", (size_t)buf->data, buf->size, buf->capacity) < 0)
    {
        strncpy(str, "(failed to write wrapped buffer status)", n);
    }
}

size_t _bio_wread(bio_data_t *bd, void *data, size_t sz)
{
    const size_t rsz = bd->buf.size - bd->offset;
    const size_t osz = rsz < sz ? rsz : sz;
    memcpy(data, (const char *)bd->buf.data + bd->offset, osz);
    bd->offset += osz;
    return osz;
}

size_t _bio_wwrite(bio_data_t *bd, const void *data, size_t sz)
{
    return buf_push(&bd->buf, data, sz);
}

void _bio_wflush(bio_data_t *bd)
{
    buf_clear(&bd->buf);
}

ssize_t _bio_wseek(bio_data_t *bd, long offset, int whence)
{
    const size_t sz = bd->buf.size;
    const size_t offset_a = offset < 0 ? -offset : offset;
    ssize_t rv;
    switch (whence)
    {
    case SEEK_SET:
        if (offset < 0 || offset_a > sz)
        {
            rv = -1;
        }
        else
        {
            bd->offset = offset_a;
            rv = (ssize_t)offset;
        }
        break;
    case SEEK_CUR:
        if (offset < 0)
        {
            if (offset_a > bd->offset)
            {
                rv = -1;
            }
            else
            {
                bd->offset -= offset_a;
                rv = (ssize_t)bd->offset;
            }
        }
        else
        {
            if (offset_a > sz - bd->offset)
            {
                rv = -1;
            }
            else
            {
                bd->offset += offset_a;
                rv = (ssize_t)bd->offset;
            }
        }
        break;
    case SEEK_END:
        if (offset > 0 || offset_a > sz)
        {
            rv = -1;
        }
        else
        {
            bd->offset = sz - offset_a;
            rv = (ssize_t)bd->offset;
        }
        break;
    default:
        rv = -1;
    }
    return rv;
}

void _bio_wdfree(bio_data_t *bd)
{
    buf_free(&bd->buf);
}

void bio_wrap(bufferedio_t *bio, buffer_t *buf)
{
    if (buf)
    {
        buf_move(&bio->data.buf, buf);
    }
    else
    {
        buf_init(&bio->data.buf, 0);
    }
    bio->data.offset = 0;
    bio->status = &_bio_wstatus;
    bio->status_str = &_bio_wstatus_str;
    bio->read = &_bio_wread;
    bio->write = &_bio_wwrite;
    bio->flush = &_bio_wflush;
    bio->seek = &_bio_wseek;
    bio->dfree = &_bio_wdfree;
}

int bio_status(const bufferedio_t *bio)
{
    return bio->status ? bio->status(&bio->data) : BIO_STATUS_INIT;
}

char *bio_status_str(bufferedio_t *bio, char *str, size_t n)
{
    if (bio->status_str)
    {
        bio->status_str(&bio->data, str, n);
    }
    else
    {
        strncpy(str, "(bio status_str unsupported)", n);
    }
    return str;
}

size_t bio_read(bufferedio_t *bio, void *data, size_t sz)
{
    size_t osz = 0;
    size_t rsz = 0;
    do
    {
        rsz = bio->read(&bio->data, (char *)data + osz, sz - osz);
        osz += rsz;
    } while (osz < sz && rsz);
    return osz;
}

const buffer_t *bio_read_all(bufferedio_t *bio, buffer_t *buf)
{
    const size_t insz = buf->size;
    const buffer_t *rv = buf;
    const ssize_t inpos = bio_seek(bio, 0, SEEK_CUR);
    if (inpos < 0)
    {
        goto seekfail;
    }
    const ssize_t endpos = bio_seek(bio, 0, SEEK_END);
    if (endpos < 0)
    {
        goto seekfail;
    }
    if (bio_seek(bio, inpos, SEEK_SET) != inpos)
    {
        /* uniquely failed to seek back to original position, will retry once */
        goto error;
    }
    size_t rsz = (size_t)(endpos - inpos);
    if (buf_resize(buf, insz + rsz) != insz + rsz)
    {
        goto error;
    }
    /* allow for potentially reading less bytes than expected from seeking */
    rsz = bio_read(bio, (char *)buf->data + insz, rsz);
    if (bio_status(bio) < BIO_STATUS_INIT)
    {
        goto error;
    }
    buf_resize(buf, insz + rsz); /* never allocating */
    goto end;
seekfail:
    /* read incrementally */
    const size_t minrsz = insz < sizeof(size_t) ? sizeof(size_t) : insz;
    do
    {
        const size_t prevsz = buf->size;
        /* scale reads by allocated memory size, limiting to ~8 reads per alloc */
        rsz = minrsz < buf->capacity / 8 ? buf->capacity / 8 : minrsz;
        if (buf_push(buf, NULL, rsz) != rsz)
        {
            goto error;
        }
        rsz = bio_read(bio, (char *)buf->data + prevsz, rsz);
        buf_resize(buf, prevsz + rsz); /* never allocating */
    } while (rsz);
    if (bio_status(bio) < BIO_STATUS_INIT)
    {
        goto error;
    }
    goto end;
error:
    rv = NULL;
    /* attempt to reset position and buffer */
    buf_resize(buf, insz);
    if (inpos >= 0)
    {
        bio_seek(bio, inpos, SEEK_SET);
    }
end:
    return rv;
}

size_t bio_write(bufferedio_t *bio, const void *data, size_t sz)
{
    size_t osz = 0;
    size_t wsz = 0;
    do
    {
        wsz = bio->write(&bio->data, (const char *)data + osz, sz - osz);
        osz += wsz;
    } while (osz < sz && wsz);
    return osz;
}

void bio_flush(bufferedio_t *bio)
{
    bio->flush(&bio->data);
}

ssize_t bio_seek(bufferedio_t *bio, long offset, int whence)
{
    return bio->seek(&bio->data, offset, whence);
}

void bio_dfree(bufferedio_t *bio)
{
    if (bio)
    {
        if (bio->dfree)
        {
            bio->dfree(&bio->data);
        }
        else
        {
            buf_free(&bio->data.buf);
            buf_free(&bio->data.opaque);
        }
    }
}