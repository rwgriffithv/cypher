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

size_t _fio_read(bio_data_t *bd, void *data, size_t sz)
{
    /*
    get bytes from internal buffer when possible
    refill buffer as needed
    */
    const size_t osz = sz;
    size_t rsz = buf_size(bd->buf) - bd->offset;
    char *src = buf_data(bd->buf);
    char *dst = data;
    while (rsz < sz)
    {
        memcpy(dst, src + bd->offset, rsz);
        sz -= rsz;
        dst += rsz;
        bd->offset = 0;
        rsz = fread(src, buf_capacity(bd->buf), 1, bd->opaque);
        buf_resize(bd->buf, rsz);
        if (rsz == 0)
        {
            /* failed to read anything at all */
            return osz - sz;
        }
    }
    memcpy(dst, src + bd->offset, sz);
    bd->offset += sz;
    return osz;
}

size_t _fio_write(bio_data_t *bd, void *data, size_t sz)
{
    /*
    write bytes to internal buffer when possible
    write buffer to file when full
    */
    const size_t osz = 0;
    const size_t cap = buf_capacity(bd->buf);
    size_t wsz = cap - buf_size(bd->buf);
    char *dst = buf_data(bd->buf);
    char *src = data;
    while (wsz < sz)
    {
        buf_push_strict(bd->buf, src, wsz);
        sz -= wsz;
        src += wsz;
        wsz = fwrite(dst + bd->offset, cap - bd->offset, 1, bd->opaque);
        bd->offset += wsz;
        if (wsz == 0)
        {
            /* failed to write anything at all */
            return osz - sz;
        }
        else if (bd->offset < cap)
        {
            /* partial write, reattempt next loop */
            wsz = 0;
        }
        else
        {
            buf_resize(bd->buf, 0);
            bd->offset = 0;
            wsz = cap;
        }
    }
    buf_push_strict(bd->buf, src, sz);
    return osz;
}

void _fio_flush(bio_data_t *bd)
{
    /* flush buffer to file (should be invoked before seek) */
    size_t wsz = 0;
    size_t sz = buf_size(bd->buf);
    char *src = buf_data(bd->buf) + bd->offset;
    do
    {
        wsz = fwrite(src, sz, 1, bd->opaque);
        src += wsz;
        sz -= wsz;
    } while (wsz > 0);
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

int fio_init(bufferedio_t *fb, size_t bufsz)
{
    fb->data.buf = buf_init(bufsz);
    fb->data.offset = 0;
    fb->data.opaque = NULL;
    fb->read = &_fio_read;
    fb->write = &_fio_write;
    fb->flush = &_fio_flush;
    fb->seek = &_fio_seek;
    return fb->data.buf ? 0 : 1;
}

int fio_open(bufferedio_t *fb, const char *path, const char *mode)
{
    fio_close(fb);
    fb->data.opaque = fopen(path, mode);
    return fb->data.opaque ? 0 : 1;
}

int fio_close(bufferedio_t *fb)
{
    int rv = EOF;
    if (fb->data.opaque)
    {
        rv = fclose(fb->data.opaque);
        fb->data.opaque = NULL;
    }
    return rv;
}

void fio_free(bufferedio_t *fb)
{
    if (fb)
    {
        fio_close(fb);
        buf_free(fb->data.buf);
    }
}