/**
 * @file cypher.c
 * @author Rob Griffith
 */

#include "cypher.h"
#include "sha256.h"

#include <stdio.h>

void print_hash(sha256hash_t *hash, const char *prefix)
{
    buffer_t *hash_str = sha256_hexstr(hash);
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

sha256hash_t *hash_key(bufferedio_t *key, sha256hash_t *out)
{
    out = sha256(key, out);
    if (!out)
    {
        fprintf(stderr, "failed to get SHA256 hash for key\n");
        return NULL;
    }
    print_hash(out, "SHA256 key hash");
    return out;
}

int cypher(bufferedio_t *bio_in, bufferedio_t *key, bufferedio_t *bio_out)
{
    sha256hash_t hash;
    if (!hash_key(key, &hash))
    {
        fprintf(stderr, "failed to hash key\n");
        return 1;
    }
    uint32_t word;
    size_t i;
    while (1)
    {
        const size_t rsz = bio_read(bio_in, &word, sizeof(word));
        word ^= hash.words[i];
        i = (i + 1) % 8;
        if (rsz && bio_write(bio_out, &word, rsz) != rsz)
        {
            break;
        }
    }
    return 0;
}