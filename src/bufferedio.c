/**
 * @file bufferedio.c
 * @author Rob Griffith
 */

#include "bufferedio.h"

#include <stdio.h>
#include <string.h>

int _bio_wstatus(const bio_data_t *bd)
{
    return bd->buf ? 1 : -1;
}

size_t _bio_wread(bio_data_t *bd, void *data, size_t sz)
{
    const size_t rsz = buf_size(bd->buf) - bd->offset;
    const size_t osz = rsz < sz ? rsz : sz;
    memcpy(data, (const char *)buf_cdata(bd->buf) + bd->offset, osz);
    bd->offset += osz;
    return osz;
}

size_t _bio_wwrite(bio_data_t *bd, const void *data, size_t sz)
{
    return buf_push(bd->buf, data, sz);
}

void _bio_wflush(bio_data_t *bd)
{
    buf_resize(bd->buf, 0);
}

int _bio_wseek(bio_data_t *bd, long offset, int whence)
{
    const size_t sz = buf_size(bd->buf);
    const size_t offset_a = offset < 0 ? -offset : offset;
    int rv = 0;
    switch (whence)
    {
    case SEEK_SET:
        if (offset < 0 || offset_a >= sz)
        {
            rv = 1;
        }
        else
        {
            bd->offset = offset_a;
        }
        break;
    case SEEK_CUR:
        if (offset < 0)
        {
            if (offset_a > bd->offset)
            {
                rv = 1;
            }
            else
            {
                bd->offset -= offset_a;
            }
        }
        else if (offset_a >= sz - bd->offset)
        {
            rv = 1;
        }
        else
        {
            bd->offset += offset_a;
        }
        break;
    case SEEK_END:
        if (offset >= 0 || offset_a > sz)
        {
            rv = 1;
        }
        else
        {
            bd->offset = sz - offset_a;
        }
        break;
    default:
        rv = 2;
    }
    return rv;
}

void _bio_wdfree(bio_data_t *bd)
{
    buf_free(bd->buf);
    bd->buf = NULL;
}

void bio_wrap(bufferedio_t *bio, buffer_t *buf)
{
    bio->data.buf = buf;
    bio->data.offset = 0;
    bio->data.opaque = NULL;
    bio->status = &_bio_wstatus;
    bio->read = &_bio_wread;
    bio->write = &_bio_wwrite;
    bio->flush = &_bio_wflush;
    bio->seek = &_bio_wseek;
    bio->dfree = &_bio_wdfree;
}

int bio_status(const bufferedio_t *bio)
{
    return bio->status(&bio->data);
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

int bio_seek(bufferedio_t *bio, long offset, int whence)
{
    return bio->seek(&bio->data, offset, whence);
}

void bio_dfree(bufferedio_t *bio)
{
    if (bio && bio->dfree)
    {
        bio->dfree(&bio->data);
    }
}