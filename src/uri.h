/**
 * @file uri.h
 * @author Rob Griffith
 */

#ifndef URI_H
#define URI_H

#include "buffer.h"

/**
 * @struct uri_comp
 * @brief syntax component of URI
 * @typedef uri_comp_t
 *
 * Beginning and terminating syntax characters should not be included.
 */
typedef struct uri_comp
{
    const char *str; /** starting point of element of URI (NULL if absent) */
    size_t len;      /** number of characters in element (no null byte) */
} uri_comp_t;

/**
 * @struct uri_authority
 * @brief context for URI naming authority
 * @typedef uri_auth_t
 */
typedef struct uri_auth
{
    uri_comp userinfo; /** user info */
    uri_comp host;     /** host */
    uri_comp port;     /** port */
} uri_auth_t;

/**
 * @struct uri
 * @brief context for Universal Resource Identifier (URI) defined in RFC 3986
 * @typedef uri_t
 *
 * Beginning and terminating syntax characters are not included.
 * Struct can be used to defined components of encoded or decoded URI.
 */
typedef struct uri
{
    const char *str;     /** entire URI string */
    uri_comp_t scheme;   /** scheme name in URI */
    uri_auth authority;  /** naming authority */
    uri_comp_t path;     /** path */
    uri_comp_t query;    /** query */
    uri_comp_t fragment; /** fragment */
} uri_t;

/**
 * @todo
 * @brief parse encoded URI string into constituent encoded components
 *
 * @param str the encoded URI string
 * @param[out] uri URI context to populate with encoded components
 * @return populated URI context, NULL if incorrectly formatted URI string
 */
const uri_t *uri_parse(const char *str, uri_t *uri);

/**
 * @todo
 * @brief encode a decoded URI into percent-encoded form
 *
 * Decoded URI must be separated into constituent components.
 * Other transcoding (e.g. for UTF-8) must occur before this step.
 * In each component (with its own set of valid subcomponent delimiters):
 *   1. preserve bytes that are unreserved or valid subcomponent delimiters
 *   2. percent-encode all other bytes
 * Constructed URI is pushed to buffer and includes component delimiters.
 *
 * @param dec decoded URI context to encode
 * @param buf buffer to construct full URI in (pushing)
 * @param[out] enc encoded URI context
 * @return entire encoded URI string, NULL if error
 */
const char *uri_encode(const uri_t *dec, buffer_t *buf, uri_t *enc);

/**
 * @todo
 * @brief decode a percent-encoded URI into decoded form
 *
 * This is an inversion of @ref uri_encode that undoes all percent-encoding.
 * Other transcoding (e.g. for UTF-8) must occur after this step.
 * Constructed URI is pushed to buffer and includes component delimiters.
 *
 * @param enc encoded URI context to decode
 * @param buf buffer to construct full URI in (pushing)
 * @param[out] dec decoded URI context
 * @return entire decoded URI string, NULL if error
 */
const char *uri_decode(const uri_t *enc, buffer_t *buf, uri_t *dec);

#endif