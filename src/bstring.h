/**
 * @file bstring.h
 * @author Rob Griffith
 */

#include "buffer.h"

#include <stdarg.h>

/**
 * @brief concatenate string onto end of buffer
 *
 * If present, will overwrite ending null byte in buffer.
 *
 * @param[inout] buf buffer holding prefix string
 * @param str suffix string
 * @return full string in buffer (NULL if failed allocation)
 */
const char *bstr_concat(buffer_t *buf, const char *str);

/**
 * @brief append formatted string into buffer
 *
 * Uses vsprintf for processing formatting.
 * If present, will overwrite ending null byte in buffer.
 *
 * @param[inout] buf buffer to append formatted string into
 * @param fmt string format
 * @param ... arguments for string format
 * @return formatted string appended in buffer (NULL if failed allocation)
 */
const char *bstr_printf(buffer_t *buf, const char *fmt, ...);

/**
 * @brief append formatted string into buffer (variable argument list)
 *
 * Uses vsprintf for processing formatting.
 * If present, will overwrite ending null byte in buffer.
 *
 * @param[inout] buf buffer to append formatted string into
 * @param fmt string format
 * @param args variable argument list for string format
 * @return formatted string appended in buffer (NULL if failed allocation)
 */
const char *bstr_vprintf(buffer_t *buf, const char *fmt, va_list args);