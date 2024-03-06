/**
 * @file fdio.h
 * @author Rob Griffith
 */

#ifndef FDIO_H
#define FDIO_H

#include "bufferedio.h"

void fdio_wrap(bufferedio_t *fdb, int fd, size_t bufsz);

#endif