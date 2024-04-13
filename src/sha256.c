/**
 * @file sha256.c
 * @author Rob Griffith
 */

#include "sha256.h"

#include <stdio.h>
#include <string.h>

uint32_t bswap_32(uint32_t in)
{
    return (in << 24) | ((in & 0xFF00) << 8) | ((in >> 8) & 0xFF00) | (in >> 24);
}

uint64_t bswap_64(uint64_t in)
{
    return (in << 56) | ((in & 0xFF00) << 40) | ((in & 0xFF0000) << 24) | ((in & 0xFF000000) << 8) | ((in >> 8) & 0xFF000000) | ((in >> 24) & 0xFF0000) | ((in >> 40) & 0xFF00) | (in >> 56);
}

uint32_t rotate_r(uint32_t val, size_t n)
{
    n = n % 32;
    return (val >> n) | (n ? (val << (32 - n)) : 0);
}

void sha256(bufferedio_t *bio, sha256hash_t *out)
{
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    const uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    uint32_t chunk[16];
    uint32_t w[64]; /* words used per-chunk */
    uint32_t a[8];  /* a, b, c, d, e, f, g, h used per-chunk */
    size_t insz = 0;
    int reading = 1;
    while (reading)
    {
        const size_t rsz = bio_read(bio, chunk, sizeof(chunk));
        if (rsz != sizeof(chunk))
        {
            uint8_t *end = (uint8_t *)chunk + rsz;
            end[0] = 0x80;                                                                 /* 1 bit that starts padding */
            const uint64_t len = bswap_64((uint64_t)(insz * 8));                           /* big-endian bitlength of original data */
            const size_t npz = sizeof(chunk) - ((insz + 1 + sizeof(len)) % sizeof(chunk)); /* number of padded zero bytes */
            memset(end + 1, 0, npz);
            memcpy(end + 1 + npz, &len, sizeof(len));
            reading = 0;
        }
        for (size_t i = 0; i < 16; ++i)
        {
            w[i] = bswap_32(chunk[i]);
        }
        for (size_t i = 16; i < 64; ++i)
        {
            const uint32_t s0 = rotate_r(w[i - 15], 7) ^ rotate_r(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const uint32_t s1 = rotate_r(w[i - 2], 17) ^ rotate_r(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        memcpy(a, h, sizeof(h));
        for (size_t i = 0; i < 64; ++i)
        {
            const uint32_t s1 = rotate_r(a[4], 6) ^ rotate_r(a[4], 11) ^ rotate_r(a[4], 25);
            const uint32_t ch = (a[4] & a[5]) ^ ((~a[4]) & a[6]);
            const uint32_t t1 = a[7] + s1 + ch + k[i] + w[i];
            const uint32_t s0 = rotate_r(a[0], 2) ^ rotate_r(a[0], 13) ^ rotate_r(a[0], 22);
            const uint32_t maj = (a[0] & a[1]) ^ (a[0] & a[2]) ^ (a[1] & a[2]);
            const uint32_t t2 = s0 + maj;
            memcpy(a + 1, a, sizeof(a) - sizeof(a[0]));
            a[0] = t1 + t2;
            a[4] += t1;
        }
        for (size_t i = 0; i < 8; ++i)
        {
            h[i] += a[i];
        }
    }
    /* store in little-endian */
    for (size_t i = 0; i < 8; ++i)
    {
        out->words[7 - i] = h[i];
    }
}

void sha256_hexstr(sha256hash_t *hash, sha256hex_t *out)
{
    char *str = out->str;
    for (size_t i = 0; i < 32; ++i, str += 2)
    {
        snprintf(str, 3, "%02X", hash->bytes[31 - i]);
    }
    *str = '\0';
}