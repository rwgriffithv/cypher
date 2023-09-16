/**
 * @file fileio.h
 * @author Rob Griffith
 */

#include "buffer.h"

buffer_h read_file(const char *path);

size_t write_file(const char *path, buffer_h buf);