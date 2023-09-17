/**
 * @file fileio.c
 * @author Rob Griffith
 */

#include "fileio.h"

#include <stdio.h>
#include <errno.h>

buffer_h read_file(const char *path)
{
    buffer_h buf = NULL;
    FILE *f = fopen(path, "r");
    if (!f)
    {
        fprintf(stderr, "failed to open file at %s\n", path);
        perror(errno);
        return buf;
    }
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "failed to seek end of file at %s\n", path);
        perror(errno);
        return buf;
    }
    const long sz = ftell(f);
    if (sz < 0)
    {
        fprintf(stderr, "failed to tell size of file at %s\n", path);
        perror(errno);
        return buf;
    }
    const size_t fsz = (size_t)sz;
    buf = buf_init(fsz);
    if (!buf)
    {
        fprintf(stderr, "failed to allocate buffer for %zu bytets of file at %s\n", fsz, path);
        perror(errno);
        return buf;
    }
    if (!fseek(f, 0, SEEK_SET))
    {
        fprintf(stderr, "failed to seek beginning of file at %s\n", path);
        perror(errno);
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
        fprintf(stderr, "failed to close file at %s\n", path);
        perror(errno);
    }
    return buf;
}

size_t write_file(const char *path, buffer_h buf)
{
    FILE *f = fopen(path, "w");
    if (!f)
    {
        fprintf(stderr, "failed to open file at %s\n", path);
        perror(errno);
        return 0;
    }
    const size_t wsz = fwrite(buf_data(buf), 1, buf_size(buf), f);
    if (wsz != buf_size(buf))
    {
        fprintf(stderr, "only wrote %zu of %zu bytes to file at %s\n", wsz, buf_size(buf), path);
    }
    if (fclose(f) != 0)
    {
        fprintf(stderr, "failed to close file at %s\n", path);
        perror(errno);
    }
    return wsz;
}