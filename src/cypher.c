/**
 * @file cypher.c
 * @author Rob Griffith
 */

#include "cypher.h"
#include "sha256.h"

#include <stdio.h>

void print_hash(sha256hash_t *hash, const char *prefix)
{
    buffer_t hash_str = {0};
    sha256_hexstr(hash, &hash_str);
    if (hash_str.data)
    {
        printf("%s: 0x%s\n", prefix, (char *)hash_str.data);
        buf_free(&hash_str);
    }
    else
    {
        fprintf(stderr, "%s: failed to print hash as hex string\n", prefix);
    }
}

int cypher(bufferedio_t *bio_in, bufferedio_t *key, bufferedio_t *bio_out)
{
    sha256hash_t hash;
    sha256(key, &hash);
    print_hash(&hash, "SHA256 key hash");
    uint32_t word;
    size_t i = 0;
    while (1)
    {
        const size_t rsz = bio_read(bio_in, &word, sizeof(word));
        word ^= hash.words[i];
        i = (i + 1) % 8;
        if (!rsz || bio_write(bio_out, &word, rsz) != rsz)
        {
            break;
        }
    }
    return 0;
}