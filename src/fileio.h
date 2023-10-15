/**
 * @file fileio.h
 * @author Rob Griffith
 */

#ifndef FILEIO_H
#define FILEIO_H

#include "bufferedio.h"

buffer_t *fio_read_all(const char *path);

size_t fio_write_all(const char *path, buffer_t *buf);

int fio_init(bufferedio_t *fb, size_t bufsz);

int fio_open(bufferedio_t *fb, const char *path, const char *mode);

int fio_close(bufferedio_t *fb);

void fio_free(bufferedio_t *fb);

#endif