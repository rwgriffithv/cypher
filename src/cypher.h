/**
 * @file cypher.h
 * @author Rob Griffith
 */

#ifndef CYPHER_H
#define CYPHER_H

#include "sha256.h"

/**
 * @brief pseudo-encrypt bytes input to bytes output using XOR with SHA256 hash
 *
 * SHA256 hash is XOR'd with input byte stream.
 * Operation terminates upon EOF from input or I/O error from input or output
 * Status of buffered I/O context should be checked for error.
 *
 * @param bio_in input bytes buffered I/O context
 * @param hash SHA256 hash to use
 * @param[inout] bio_out output bytes buffered I/O context
 * @return number of bytes written to output
 */
size_t cypher_xor(bufferedio_t *bio_in, sha256hash_t *hash, bufferedio_t *bio_out);

#endif