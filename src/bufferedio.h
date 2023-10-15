/**
 * @file bufferedio.h
 * @author Rob Griffith
 */

#ifndef BUFFEREDIO_H
#define BUFFEREDIO_H

#include "buffer.h"

typedef struct bio_data
{
    buffer_t *buf;
    size_t offset;
    void *opaque;
} bio_data_t;

typedef struct bufferedio
{
    bio_data_t data;
    size_t (*read)(bio_data_t *, void *, size_t);
    size_t (*write)(bio_data_t *, void *, size_t);
    void (*flush)(bio_data_t *);
    int (*seek)(bio_data_t *, long, int);
} bufferedio_t;

void bio_wrap(bufferedio_t *bio, buffer_t *buf);

size_t bio_read(bufferedio_t *bio, void *data, size_t sz);

size_t bio_write(bufferedio_t *bio, void *data, size_t sz);

void bio_flush(bufferedio_t *bio);

int bio_seek(bufferedio_t *bio, long offset, int whence);

#endif