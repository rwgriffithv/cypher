/**
 * @file sha256.h
 * @author Rob Griffith
 */

#ifndef SHA256_H
#define SHA256_H

#include "bufferedio.h"

#include <stdint.h>

/**
 * @union sha256hash
 * @brief store bytes comprising SHA256 256-bit hash
 * @typedef sha256hash_t
 */
typedef union sha256hash
{
    uint8_t bytes[32]; /** the allocated bytes */
    uint32_t words[8]; /** the allocated words */
} sha256hash_t;

/**
 * @struct sha256hex
 * @brief hexadecimal character string of SHA256 hash
 * @typedef sha256hex_t
 */
typedef struct sha256hex
{
    char str[65]; /** null-terminated hex string of hash */
} sha256hex_t;

/**
 * @brief create SHA256 hash of input bytes
 *
 * Operation terminates upon EOF or I/O error.
 *
 * @param bio input bytes buffered I/O context
 * @param[inout] out SHA256 hash
 */
void sha256(bufferedio_t *bio, sha256hash_t *out);

/**
 * @brief create hexadecimal character string of SHA256 hash
 *
 * @param hash SHA256 hash
 * @param[out] out hex string representation of SHA256 hash
 * @return pointer to beginning of null-terminated string
 */
const char *sha256_hexstr(sha256hash_t *hash, sha256hex_t *out);

#endif