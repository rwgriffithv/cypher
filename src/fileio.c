/**
 * @file fileio.c
 * @author Rob Griffith
 */

#include "fileio.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

int fio_read_all(buffer_t *buf, const char *path)
{
    int rv = 0;
    size_t rsz = 0;
    FILE *f = fopen(path, "r");
    if (!f)
    {
        fprintf(stderr, "failed to open file at %s: %s\n", path, strerror(errno));
        goto error;
    }
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "failed to seek end of file at %s: %s\n", path, strerror(errno));
        goto error;
    }
    const long sz = ftell(f);
    if (sz < 0)
    {
        fprintf(stderr, "failed to tell size of file at %s: %s\n", path, strerror(errno));
        goto error;
    }
    const size_t fsz = (size_t)sz;
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "failed to seek back to beginning of file at %s: %s\n", path, strerror(errno));
        goto error;
    }
    if (fsz)
    {
        if (!buf_resize(buf, fsz))
        {
            fprintf(stderr, "failed to allocate buffer for %zu bytets of file at %s: %s\n", fsz, path, strerror(errno));
            goto error;
        }
        rsz = fread(buf->data, 1, fsz, f);
        if (rsz != fsz)
        {
            fprintf(stderr, "only read %zu of %zu bytes of file at %s\n", rsz, fsz, path);
            buf_resize(buf, rsz);
        }
    }
    goto end;
error:
    rv = -1;
end:
    if (f && fclose(f) != 0)
    {
        fprintf(stderr, "failed to close file at %s: %s\n", path, strerror(errno));
    }
    return rv;
}

size_t fio_write_all(const char *path, const buffer_t *buf)
{
    size_t wsz = 0;
    FILE *f = fopen(path, "w");
    if (!f)
    {
        fprintf(stderr, "failed to open file at %s: %s\n", path, strerror(errno));
        goto end;
    }
    wsz = fwrite(buf->data, 1, buf->size, f);
    if (wsz != buf->size)
    {
        fprintf(stderr, "only wrote %zu of %zu bytes to file at %s\n", wsz, buf->size, path);
    }
end:
    if (f && fclose(f) != 0)
    {
        fprintf(stderr, "failed to close file at %s: %s\n", path, strerror(errno));
    }
    return wsz;
}