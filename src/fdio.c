/**
 * @file fileio.c
 * @author Rob Griffith
 */

#include "fileio.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const int *_fdio_fd(const bio_data_t *fdbd)
{
    return (const int *)fdbd->opaque.data;
}

int _fdio_status(const bio_data_t *bd)
{
    const int *fd = _fdio_fd(bd);
    return (bd->buf.data && fd && (*fd >= 0)) ? 1 : -1;
}

size_t _fdio_read(bio_data_t *bd, void *data, size_t sz)
{
    /*
    get bytes from internal buffer when possible
    refill buffer as needed (only once)
    relies on bio_read re-trying until complete or 0
    up to caller of bio_read to check errno
    */
    size_t rsz = bd->buf.size - bd->offset;
    if (!rsz)
    {
        /* attempt to refill buffer */
        bd->offset = 0;
        const ssize_t rv = read(*_fdio_fd(bd), bd->buf.data, bd->buf.capacity);
        rsz = rv < 0 ? 0 : (size_t)rv;
        buf_resize(&bd->buf, rsz);
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
    const size_t cap = bd->buf.capacity;
    size_t wsz = cap - bd->buf.size;
    if (!wsz)
    {
        /* attempt to empty buffer */
        const ssize_t rv = write(*_fdio_fd(bd), (const char *)bd->buf.data + bd->offset, bd->buf.size - bd->offset);
        bd->offset += rv < 0 ? 0 : (size_t)rv;
        if (bd->offset < cap)
        {
            /* failed to write entire buffer */
            return 0;
        }
        buf_resize(&bd->buf, 0);
        bd->offset = 0;
        wsz = cap;
    }
    const size_t osz = wsz < sz ? wsz : sz;
    return buf_push_strict(&bd->buf, data, osz);
}

void _fdio_flush(bio_data_t *bd)
{
    /* flush buffer (should be invoked before seek) */
    write(*_fdio_fd(bd), (const char *)bd->buf.data + bd->offset, bd->buf.size - bd->offset);
    buf_resize(&bd->buf, 0);
    bd->offset = 0;
}

int _fdio_seek(bio_data_t *bd, long offset, int whence)
{
    /* not much choice, drop buffer on the floor */
    buf_resize(&bd->buf, 0);
    bd->offset = 0;
    long rv = lseek(*_fdio_fd(bd), offset, whence);
    return rv >= 0 ? 0 : -1;
}

void _fdio_dfree(bio_data_t *bd)
{
    const int *fd = _fdio_fd(bd);
    if (fd)
    {
        if (bd->buf.data)
        {
            _fdio_flush(bd);
        }
        close(*fd);
    }
    buf_free(&bd->opaque);
    buf_free(&bd->buf);
}

void fdio_wrap(bufferedio_t *fdb, int fd, size_t bufsz)
{
    buf_init(&fdb->data.buf, bufsz);
    fdb->data.offset = 0;
    buf_copy(&fdb->data.opaque, &fd, sizeof(fd));
    fdb->status = &_fdio_status;
    fdb->read = &_fdio_read;
    fdb->write = &_fdio_write;
    fdb->flush = &_fdio_flush;
    fdb->seek = &_fdio_seek;
    fdb->dfree = &_fdio_dfree;
}