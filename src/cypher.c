/**
 * @file cypher.c
 * @author Rob Griffith
 */

#include "cypher.h"
#include "sha256.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

void print_hash(sha256hash_t *hash, const char *prefix)
{
    buffer_h hash_str = sha256_hexstr(hash);
    if (hash_str)
    {
        printf("%s: 0x%s\n", prefix, buf_data(hash_str));
        buf_free(hash_str);
    }
    else
    {
        fprintf(stderr, "%s: failed to print hash as hex string\n", prefix);
    }
}

uint32_t get_spice(sha256hash_t *hash)
{
    return (hash->words[0] + hash->words[5] + hash->words[0] + hash->words[7]);
}

sha256hash_t *hash_key(const char *key, sha256hash_t *out)
{
    buffer_h buf_key = buf_copy(key, strlen(key));
    if (!buf_key)
    {
        fprintf(stderr, "failed to make buffer for key hash: %s\n", strerror(errno));
        return NULL;
    }
    out = sha256(buf_key, out);
    buf_free(buf_key);
    if (!out)
    {
        fprintf(stderr, "failed to get SHA256 hash for key\n");
        return NULL;
    }
    print_hash(out, "SHA256 key hash");
    const uint32_t s = get_spice(out);
    for (size_t i = 0; i < 8; ++i)
    {
        out->words[i] ^= s;
    }
    print_hash(out, "final key hash");
    return out;
}

buffer_h cypher(buffer_h buf, const char *key)
{
    const size_t sz = buf_size(buf);
    if (!sz)
    {
        return buf;
    }
    sha256hash_t hash;
    if (!hash_key(key, &hash))
    {
        fprintf(stderr, "failed to hash key\n");
        return NULL;
    }
    /* obfuscate where hash application begins */
    const size_t nw = sz / sizeof(uint32_t); /* number of words */
    const size_t offset = nw ? (get_spice(&hash) % nw) : 0;
    uint32_t *b32 = buf_data(buf);
    size_t i, j;
    /* starting at offset based on hash */
    for (i = offset, j = 0; i < nw; ++i, j = (j + 1) % 8)
    {
        b32[i] ^= hash.words[j];
    }
    /* byte-by-byte for end of buffer within one word (partial hash word reused later) */
    uint8_t *b8 = buf_data(buf);
    for (i *= sizeof(uint32_t), j *= sizeof(uint32_t); i < sz; ++i, ++j)
    {
        b8[i] ^= hash.bytes[j];
    }
    /* continuing with remaining words at beginning of buffer */
    for (i = 0, j /= sizeof(uint32_t); i < offset; ++i, j = (j + 1) % 8)
    {
        b32[i] ^= hash.words[j];
    }
    /* byte-by-byte until end of hash (ensure full hash usage + obfuscation) */
    for (i *= sizeof(uint32_t), j *= sizeof(uint32_t); j < sizeof(hash.bytes); i = (i + 1) % sz, ++j)
    {
        b8[i] ^= hash.bytes[j];
    }
    return buf;
}