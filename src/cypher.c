/**
 * @file cypher.c
 * @author Rob Griffith
 */

#include "cypher.h"

#include <stdio.h>

size_t cypher_xor(bufferedio_t *bio_in, sha256hash_t *hash, bufferedio_t *bio_out)
{
    size_t rv = 0;
    size_t i = 0;
    while (1)
    {
        uint32_t word;
        const size_t rsz = bio_read(bio_in, &word, sizeof(word));
        i = (i + 1) % 8;
        word ^= hash->words[i];
        const size_t wsz = rsz ? bio_write(bio_out, &word, rsz) : 0;
        rv += wsz;
        if (!(wsz && wsz == rsz))
        {
            break;
        }
    }
    return rv;
}