/**
 * @file fileio.c
 * @author Rob Griffith
 */

#include "fileio.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct _fdio_opqd
{
    int fd;
    int err;
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

void _fdio_strstatus(const bio_data_t *bd, char *str, size_t n)
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
        strncpy(strstart, "(fdio strstatus failed to format string)", nfull);
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
    if (!rsz)
    {
        /* attempt to refill buffer */
        bd->offset = 0;
        const ssize_t rv = read(opqd->fd, bd->buf.data, bd->buf.capacity);
        opqd->err = rv < 0 ? errno : 0;
        rsz = buf_resize(&bd->buf, rv < 0 ? 0 : (size_t)rv);
    }
    const size_t osz = rsz < sz ? rsz : sz;
    memcpy(data, (const char *)bd->buf.data + bd->offset, osz);
    bd->offset += osz;
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
    if (!wsz)
    {
        /* attempt to empty buffer */
        const ssize_t rv = write(opqd->fd, (const char *)bd->buf.data + bd->offset, bd->buf.size - bd->offset);
        opqd->err = rv < 0 ? errno : 0;
        bd->offset += rv < 0 ? 0 : (size_t)rv;
        if (bd->offset < cap)
        {
            /* failed to write entire buffer */
            return 0;
        }
        buf_clear(&bd->buf);
        bd->offset = 0;
        wsz = cap;
    }
    const size_t osz = wsz < sz ? wsz : sz;
    return buf_push(&bd->buf, data, osz); /* should never increase capacity */
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

int _fdio_seek(bio_data_t *bd, long offset, int whence)
{
    /* not much choice, drop buffer on the floor */
    buf_clear(&bd->buf);
    bd->offset = 0;
    _fdio_opqd_t *opqd = bd->opaque.data;
    long rv = lseek(opqd->fd, offset, whence);
    opqd->err = rv == (long)-1 ? errno : 0;
    return rv == (long)-1 ? -1 : 0;
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
        close(opqd->fd);
    }
    buf_free(&bd->opaque);
    buf_free(&bd->buf);
}

void fdio_wrap(bufferedio_t *fdb, int fd, size_t bufsz)
{
    buf_init(&fdb->data.buf, bufsz);
    fdb->data.offset = 0;
    _fdio_opqd_t opqd = {fd, fd < 0 ? errno : 0};
    buf_copy(&fdb->data.opaque, &opqd, sizeof(_fdio_opqd_t));
    fdb->status = &_fdio_status;
    fdb->strstatus = &_fdio_strstatus;
    fdb->read = &_fdio_read;
    fdb->write = &_fdio_write;
    fdb->flush = &_fdio_flush;
    fdb->seek = &_fdio_seek;
    fdb->dfree = &_fdio_dfree;
}