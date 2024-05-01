/**
 * @file fdio.c
 * @author Rob Griffith
 */

#include "fdio.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct _fdio_opqd
{
    int fd;
    int err;
    int clfags;
} _fdio_opqd_t;

int _fdio_status(const bio_data_t *bd)
{
    const _fdio_opqd_t *opqd = bd->opaque.data;
    if (opqd && opqd->err)
    {
        return BIO_STATUS_INIT - opqd->err;
    }
    return BIO_STATUS_INIT + ((bd->buf.data && opqd && (opqd->fd >= 0)) ? 1 : 0);
}

void _fdio_status_str(const bio_data_t *bd, char *str, size_t n)
{
    char *strstart = str;
    size_t nfull = n;
    const _fdio_opqd_t *opqd = bd->opaque.data;
    if (!opqd)
    {
        strncpy(str, "uninitialized, no file descriptor", n);
        return;
    }
    int rv = snprintf(str, n, "{fd: %d, bufsz: %zu}", opqd->fd, bd->buf.capacity);
    if (rv < 0)
    {
        goto end;
    }
    n -= (size_t)rv;
    str += (size_t)rv;
    if (opqd->err)
    {
        rv = snprintf(str, n, ", error: %s", strerror(opqd->err));
    }
end:
    if (rv < 0)
    {
        memset(strstart, 0, nfull);
        strncpy(strstart, "(fdio status_str failed to format string)", nfull);
    }
}

size_t _fdio_read(bio_data_t *bd, void *data, size_t sz)
{
    /*
    get bytes from internal buffer when possible
    refill buffer as needed (only once)
    relies on bio_read re-trying until complete or 0
    up to caller of bio_read to check errno
    */
    _fdio_opqd_t *opqd = bd->opaque.data;
    size_t rsz = bd->buf.size - bd->offset;
    size_t osz;
    if (sz > rsz + bd->buf.capacity)
    {
        /* drain buffer and read directly */
        memcpy(data, (const char *)bd->buf.data + bd->offset, rsz);
        bd->offset = 0;
        buf_clear(&bd->buf);
        const ssize_t rv = read(opqd->fd, (char *)data + rsz, sz - rsz);
        opqd->err = rv < 0 ? errno : 0;
        osz = rsz + (rv < 0 ? 0 : (size_t)rv);
    }
    else
    {
        if (!rsz)
        {
            /* refill buffer */
            bd->offset = 0;
            const ssize_t rv = read(opqd->fd, bd->buf.data, bd->buf.capacity);
            opqd->err = rv < 0 ? errno : 0;
            rsz = buf_resize(&bd->buf, rv < 0 ? 0 : (size_t)rv);
        }
        osz = rsz < sz ? rsz : sz;
        memcpy(data, (const char *)bd->buf.data + bd->offset, osz);
        bd->offset += osz;
    }
    return osz;
}

size_t _fdio_write(bio_data_t *bd, const void *data, size_t sz)
{
    /*
    write bytes to internal buffer when possible
    write buffer to file when full (only once)
    relies on bio_write re-trying until complete or 0
    up to caller of bio_read to check errno
    */
    _fdio_opqd_t *opqd = bd->opaque.data;
    const size_t cap = bd->buf.capacity;
    size_t wsz = cap - bd->buf.size;
    size_t osz;
    if (!wsz || sz > wsz + cap)
    {
        /* flush buffer */
        ssize_t rv = write(opqd->fd, (const char *)bd->buf.data + bd->offset, bd->buf.size - bd->offset);
        opqd->err = rv < 0 ? errno : 0;
        bd->offset += rv < 0 ? 0 : (size_t)rv;
        if (bd->offset < bd->buf.size)
        {
            /* failed to write entire buffer */
            osz = 0;
            goto end;
        }
        buf_clear(&bd->buf);
        bd->offset = 0;
        if (sz > wsz + cap)
        {
            /* write directly */
            rv = write(opqd->fd, data, sz);
            opqd->err = rv < 0 ? errno : 0;
            osz = rv < 0 ? 0 : (size_t)rv;
            goto end;
        }
        wsz = cap;
    }
    /* push to buffer, never increasing capacity */
    osz = buf_push(&bd->buf, data, wsz < sz ? wsz : sz);
end:
    return osz;
}

void _fdio_flush(bio_data_t *bd)
{
    /* flush buffer (should be invoked before seek) */
    _fdio_opqd_t *opqd = bd->opaque.data;
    const ssize_t rv = write(opqd->fd, (const char *)bd->buf.data + bd->offset, bd->buf.size - bd->offset);
    opqd->err = rv < 0 ? errno : 0;
    buf_clear(&bd->buf);
    bd->offset = 0;
}

ssize_t _fdio_seek(bio_data_t *bd, long offset, int whence)
{
    /* not much choice, drop buffer on the floor */
    buf_clear(&bd->buf);
    bd->offset = 0;
    _fdio_opqd_t *opqd = bd->opaque.data;
    off_t rv = lseek(opqd->fd, offset, whence);
    opqd->err = rv == (long)-1 ? errno : 0;
    return (ssize_t)rv;
}

void _fdio_dfree(bio_data_t *bd)
{
    _fdio_opqd_t *opqd = bd->opaque.data;
    if (opqd)
    {
        if (bd->buf.data)
        {
            _fdio_flush(bd);
        }
        if (opqd->clfags & FDIO_CLOSE)
        {
            close(opqd->fd);
        }
    }
    buf_free(&bd->opaque);
    buf_free(&bd->buf);
}

void fdio_wrap(bufferedio_t *fdb, int fd, size_t bufsz, int cflags)
{
    buf_init(&fdb->data.buf, bufsz);
    fdb->data.offset = 0;
    _fdio_opqd_t opqd = {fd, fd < 0 ? errno : 0, cflags};
    buf_copy(&fdb->data.opaque, &opqd, sizeof(_fdio_opqd_t));
    fdb->status = &_fdio_status;
    fdb->status_str = &_fdio_status_str;
    fdb->read = &_fdio_read;
    fdb->write = &_fdio_write;
    fdb->flush = &_fdio_flush;
    fdb->seek = &_fdio_seek;
    fdb->dfree = &_fdio_dfree;
}