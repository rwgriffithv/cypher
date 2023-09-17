/**
 * @file cypher.c
 * @author Rob Griffith
 */

#include "cypher.h"
#include "sha256.h"

#include <stdio.h>
#include <string.h>

sha256hash_t *hash_key(char *key, sha256hash_t *out)
{
    buffer_h buf_key = buf_copy(key, strlen(key));
    if (!buf_key)
    {
        fprintf(stderr, "failed to make buffer for key hashing\n");
        return NULL;
    }
    out = sha256(buf_key, out);
    buf_free(buf_key);
    buffer_h hash_str = sha256_hexstr(out);
    if (hash_str)
    {
        printf("original hash string: %s\n", (char *)buf_data(hash_str));
        buf_free(hash_str);
    }
    else
    {
        fprintf(stderr, "failed to get original hash string to print\n");
    }
    const uint32_t s = out->words[0] + out->words[5] + out->words[0] + out->words[7];
    for (size_t i = 0; i < 8; ++i)
    {
        out->words[i] ^= s;
    }
    hash_str = sha256_hexstr(out);
    if (hash_str)
    {
        printf("final hash string: %s\n", (char *)buf_data(hash_str));
        buf_free(hash_str);
    }
    else
    {
        fprintf(stderr, "failed to get final hash string to print\n");
    }
    return out;
}

buffer_h cypher(buffer_h buf, const char *key)
{
    sha256hash_t hash;
    if (!hash_key(key, &hash))
    {
        fprintf(stderr, "failed to hash key\n");
        return NULL;
    }
    uint32_t *b32 = (uint32_t *)buf_data(buf);
    size_t i;
    for (i = 0; i < buf_size(buf) / sizeof(uint32_t); ++i)
    {
        b32[i] ^= hash.words[i % 8];
    }
    uint8_t *b8 = (uint8_t *)(b32 + i);
    const uint8_t *hwb = (uint8_t *)(hash.words + (i % 8));
    for (i = 0; i < buf_size(buf) % sizeof(uint32_t); ++i)
    {
        b8[i] ^= hwb[i];
    }
    return buf;
}