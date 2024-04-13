/**
 * @file cypher.c
 * @author Rob Griffith
 */

#include "cypher.h"
#include "sha256.h"

#include <stdio.h>

int cypher(bufferedio_t *bio_in, bufferedio_t *key, bufferedio_t *bio_out)
{
    sha256hash_t hash;
    sha256hex_t hash_str;
    sha256(key, &hash);
    sha256_hexstr(&hash, &hash_str);
    printf("SHA256 key hash: 0x%s\n", hash_str.str);
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