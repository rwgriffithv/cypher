/**
 * @file fileio.c
 * @author Rob Griffith
 */

#include "fileio.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

buffer_t *fio_read_all(const char *path)
{
    buffer_t *buf = NULL;
    FILE *f = fopen(path, "r");
    if (!f)
    {
        fprintf(stderr, "failed to open file at %s: %s\n", path, strerror(errno));
        return buf;
    }
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "failed to seek end of file at %s: %s\n", path, strerror(errno));
        return buf;
    }
    const long sz = ftell(f);
    if (sz < 0)
    {
        fprintf(stderr, "failed to tell size of file at %s: %s\n", path, strerror(errno));
        return buf;
    }
    const size_t fsz = (size_t)sz;
    buf = buf_init(fsz);
    if (!buf)
    {
        fprintf(stderr, "failed to allocate buffer for %zu bytets of file at %s: %s\n", fsz, path, strerror(errno));
        return buf;
    }
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "failed to seek beginning of file at %s: %s\n", path, strerror(errno));
        buf_free(buf);
        return NULL;
    }
    const size_t rsz = fread(buf_data(buf), 1, fsz, f);
    if (rsz != fsz)
    {
        fprintf(stderr, "only read %zu of %zu bytes of file at %s\n", rsz, fsz, path);
        buf_resize(buf, rsz);
    }
    if (fclose(f) != 0)
    {
        fprintf(stderr, "failed to close file at %s: %s\n", path, strerror(errno));
    }
    return buf;
}

size_t fio_write_all(const char *path, buffer_t *buf)
{
    FILE *f = fopen(path, "w");
    if (!f)
    {
        fprintf(stderr, "failed to open file at %s: %s\n", path, strerror(errno));
        return 0;
    }
    const size_t wsz = fwrite(buf_data(buf), 1, buf_size(buf), f);
    if (wsz != buf_size(buf))
    {
        fprintf(stderr, "only wrote %zu of %zu bytes to file at %s\n", wsz, buf_size(buf), path);
    }
    if (fclose(f) != 0)
    {
        fprintf(stderr, "failed to close file at %s: %s\n", path, strerror(errno));
    }
    return wsz;
}

int _fio_status(const bio_data_t *bd)
{
    int rv = -1;
    if (bd->buf)
    {
        rv = bd->opaque ? 1 : 0;
    }
    return rv;
}

size_t _fio_read(bio_data_t *bd, void *data, size_t sz)
{
    /*
    get bytes from internal buffer when possible
    refill buffer as needed (only once)
    relies on bio_read re-trying until complete or 0
    */
    size_t rsz = buf_size(bd->buf) - bd->offset;
    if (!rsz)
    {
        /* attempt to refill buffer */
        bd->offset = 0;
        rsz = fread(buf_data(bd->buf), 1, buf_capacity(bd->buf), bd->opaque);
        buf_resize(bd->buf, rsz);
    }
    const size_t osz = rsz < sz ? rsz : sz;
    memcpy(data, (const char *)buf_cdata(bd->buf) + bd->offset, osz);
    bd->offset += osz;
    return osz;
}

size_t _fio_write(bio_data_t *bd, const void *data, size_t sz)
{
    /*
    write bytes to internal buffer when possible
    write buffer to file when full (only once)
    relies on bio_write re-trying until complete or 0
    */
    const size_t cap = buf_capacity(bd->buf);
    size_t wsz = cap - buf_size(bd->buf);
    if (!wsz)
    {
        /* attempt to empty buffer */
        bd->offset += fwrite((const char *)buf_cdata(bd->buf) + bd->offset, 1, buf_size(bd->buf) - bd->offset, bd->opaque);
        if (bd->offset < cap)
        {
            /* failed to write entire buffer */
            return 0;
        }
        buf_resize(bd->buf, 0);
        bd->offset = 0;
        wsz = cap;
    }
    const size_t osz = wsz < sz ? wsz : sz;
    return buf_push_strict(bd->buf, data, osz);
}

void _fio_flush(bio_data_t *bd)
{
    /* flush buffer to file (should be invoked before seek) */
    fwrite((const char *)buf_cdata(bd->buf) + bd->offset, 1, buf_size(bd->buf) - bd->offset, bd->opaque);
    buf_resize(bd->buf, 0);
    bd->offset = 0;
}

int _fio_seek(bio_data_t *bd, long offset, int whence)
{
    /* not much choice, drop buffer on the floor */
    buf_resize(bd->buf, 0);
    bd->offset = 0;
    return fseek(bd->opaque, offset, whence);
}

void _fio_dfree(bio_data_t *bd)
{
    if (bd->buf && bd->opaque)
    {
        _fio_flush(bd);
        fclose(bd->opaque);
        bd->opaque = NULL;
    }
    buf_free(bd->buf);
    bd->buf = NULL;
}

int fio_init(bufferedio_t *fb, size_t bufsz)
{
    fb->data.buf = buf_init(bufsz);
    fb->data.offset = 0;
    fb->data.opaque = NULL;
    fb->status = &_fio_status;
    fb->read = &_fio_read;
    fb->write = &_fio_write;
    fb->flush = &_fio_flush;
    fb->seek = &_fio_seek;
    fb->dfree = &_fio_dfree;
    int rv = 0;
    if (fb->data.buf)
    {
        buf_resize(fb->data.buf, 0);
        rv = 1;
    }
    return rv;
}

int fio_open(bufferedio_t *fb, const char *path, const char *mode)
{
    fio_close(fb);
    fb->data.opaque = fopen(path, mode);
    return fb->data.opaque ? 0 : 1;
}

int fio_close(bufferedio_t *fb)
{
    int rv = 0;
    if (fb->data.opaque)
    {
        bio_flush(fb);
        rv = fclose(fb->data.opaque);
        fb->data.opaque = NULL;
    }
    return rv;
}