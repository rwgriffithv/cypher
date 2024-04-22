/**
 * @file tokenize.h
 * @author Rob Griffith
 */

#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "bufferedio.h"

/**
 * @brief parse token from buffered I/O context
 *
 * A single token is determined by non-whitespace characters followed by whitespace.
 * The terminating whitespace character is provided as output to help distinguish tokens.
 * All tokens are stored as null-terminated strings.
 * Empty string is only returned if EOF is reached without finding any non-whitespace characters.
 * If error, NULL is returned, end is not set, and buffered I/O context status should be checked.
 *
 * @param[inout] bio buffered I/O context
 * @param[inout] buf buffer to store (push) token to
 * @param[inout] end the whitespace character marking the end of the token (or EOF)
 * @return parsed token string (stored in buffer), NULL if failed to parse
 */
const char *tkz_parse_str_token(bufferedio_t *bio, buffer_t *buf, char *end);

#endif