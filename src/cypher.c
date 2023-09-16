/**
 * @file cypher.c
 * @author Rob Griffith
 */

#include "cypher.h"
#include "sha256.h"

#include <string.h>

buffer_h decode(buffer_h buf, char *key)
{
    buffer_h buf_key = buf_copy(key, strlen(key));
    sha256hash_t hash;
    sha256(buf_key, &hash);
    buffer_h dec = buf_init(buf_size(buf));
}

buffer_h encode(buffer_h buf, char *key)
{
    buffer_h enc = buf_init(buf_size(buf));
}