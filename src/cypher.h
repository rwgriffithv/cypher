/**
 * @file cypher.h
 * @author Rob Griffith
 */

#ifndef CYPHER_H
#define CYPHER_H

#include "bufferedio.h"

int cypher(bufferedio_t *bio_in, bufferedio_t *key, bufferedio_t *bio_out);

#endif