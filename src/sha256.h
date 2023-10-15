/**
 * @file sha256.h
 * @author Rob Griffith
 */

#ifndef SHA256_H
#define SHA256_H

#include "bufferedio.h"

#include <stdint.h>

typedef union sha256hash
{
    uint8_t bytes[32];
    uint32_t words[8];
} sha256hash_t;

sha256hash_t *sha256(bufferedio_t *bio, sha256hash_t *out);

buffer_t *sha256_hexstr(sha256hash_t *hash);

#endif