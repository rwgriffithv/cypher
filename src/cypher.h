/**
 * @file cypher.h
 * @author Rob Griffith
 */

#include "buffer.h"

buffer_h decode(buffer_h buf, const char *key);

buffer_h encode(buffer_h buf, const char *key);