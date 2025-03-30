/**
 * @file uri.c
 * @author Rob Griffith
 */

#include "uri.h"

const uri_t *uri_parse(const char *str, uri_t *uri)
{
    /** @todo use strschema.[h|c] to define schema and parse */
}

const char *uri_encode(const uri_t *dec, buffer_t *buf, uri_t *enc)
{
}

const char *uri_decode(const uri_t *enc, buffer_t *buf, uri_t *dec)
{
}