/**
 * @file sha256.c
 * @author Rob Griffith
 */

#include "sha256.h"

#include <stdio.h>
#include <string.h>

uint64_t bitswap(uint64_t in)
{
    const uint64_t mask = 1;
    uint64_t out = 0;
    for (size_t i = 0; i < 8; ++i)
    {
        out = (out << 1) | (in & mask);
        in = in >> 1;
    }
    return out;
}

uint32_t rotate_r(uint32_t val, size_t n)
{
    const uint32_t mask = 1;
    const uint32_t hi = 0x80000000;
    for (size_t i = 0; i < n; ++i)
    {
        val = (val & mask) ? ((val >> 1) | hi) : (val >> 1);
    }
    return val;
}

sha256hash_t *sha256(buffer_h buf, sha256hash_t *out)
{
    const size_t insz = buf_size(buf);
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
    uint8_t end[64] = {0};
    end[0] = 0x80;                                                             /* 1 bit that starts padding */
    const uint64_t len = bitswap((uint64_t)insz);                              /* big-endian length of original data */
    const size_t npz = sizeof(end) - ((insz + 1 + sizeof(len)) % sizeof(end)); /* number of padded zero bytes */
    const size_t apsz = 1 + npz + sizeof(len);                                 /* total number of bytes to append */
    memcpy(end + 1 + npz, &len, sizeof(len));
    if (!buf_push_strict(buf, end, apsz)) /* copy padding and length to end of in (reset at end) */
    {
        fprintf(stderr, "failed to allocate memory for %zu bytes for sha256 to append original data\n", apsz);
        return NULL;
    }
    const size_t nchunks = buf_size(buf) / 64; /* number of 512 bit chunks */
    uint8_t *chunk = buf_data(buf);
    uint32_t w[64]; /* words used per-chunk */
    uint32_t a[8];  /* a, b, c, d, e, f, g, h used per-chunk */
    for (size_t c = 0; c < nchunks; ++c, chunk += 64)
    {
        memcpy(chunk, w, 16 * sizeof(w[0]));
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
            for (size_t j = 7; j > 0; --j)
            {
                a[j] = a[j - 1];
            }
            a[0] = t1 + t2;
            a[4] += t1;
        }
        for (size_t i = 0; i < 8; ++i)
        {
            h[i] += a[i];
        }
    }
    buf_resize(buf, insz);
    memcpy(out->words, h, sizeof(h));
    return out;
}

buffer_h sha256_hexstr(sha256hash_t *hash)
{
    const size_t n = 2 * sizeof(hash->words) + 1; /* null terminated */
    buffer_h buf = buf_init(n);
    if (!buf)
    {
        fprintf(stderr, "failed to make buffer for sha256 hex string\n");
        return NULL;
    }
    char *str = (char *)buf_data(buf);
    const size_t nw = 2 * sizeof(hash->words[0]);
    for (size_t i = 0; i < 8; ++i, str += nw)
    {
        snprintf(str, nw, "%0*X", nw, hash->words[i]);
    }
    *str = '\0';
    return buf;
}