/**
 * @file fileio.h
 * @author Rob Griffith
 */

#ifndef FILEIO_H
#define FILEIO_H

#include "bufferedio.h"

size_t fio_read_all(buffer_t *buf, const char *path);

size_t fio_write_all(const char *path, buffer_t *buf);

#endif