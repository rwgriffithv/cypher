/**
 * @file sha256.h
 * @author Rob Griffith
 */

#include "buffer.h"

#include <stdint.h>

typedef struct sha256hash
{
    uint32_t words[8];
} sha256hash_t;

sha256hash_t *sha256(buffer_h buf, sha256hash_t *out);

buffer_h sha256_hexstr(sha256hash_t *hash);