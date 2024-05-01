/**
 * @file log.h
 * @author Rob Griffith
 */

#include "bufferedio.h"

/**
 * @enum log_lvl
 * @brief logging level (applies standard prefix to log)
 * @typedef log_lvl_t
 */
typedef enum log_lvl
{
    LOG_INFO,    /** general information */
    LOG_WARNING, /** deviation from normal operation */
    LOG_ERROR    /** significant fault prohibiting succesful operation */
} log_lvl_t;

/**
 * @struct log
 * @brief logging context
 * @typedef log_t
 */
typedef struct log
{
    bufferedio_t out; /** buffered I/O context for log output destination */
    buffer_t buf;     /** buffer for formatting strings */
} log_t;

/**
 * @brief get string representation of logging level
 *
 * @param lvl logging level
 * @return string representation of logging level
 */
const char *log_lvlstr(log_lvl_t lvl);

/**
 * @brief log formatted string directly
 *
 * Uses vsprintf for processing formatting.
 *
 * @param[inout] log logging context
 * @param fmt log string format
 * @param ... arguments for log string format
 * @return logged string stored in log context buffer (NULL if error)
 */
const char *log_printf(log_t *log, const char *fmt, ...);

/**
 * @brief log long formatted string with all log info
 *
 * Uses vsprintf for processing formatting.
 * Includes a timestamp, file, line number, and log level before the string.
 *
 * @param[inout] log logging context
 * @param file filename (__FILE__)
 * @param line line number (__LINE__)
 * @param lvl logging level
 * @param fmt log string format
 * @param ... arguments for log string format
 * @return logged string stored in log context buffer (NULL if error)
 */
const char *log_printf_long(log_t *log, const char *file, int line, log_lvl_t lvl, const char *fmt, ...);

/**
 * @def logfl
 * @brief alias of @ref logf_long auto-populating file and line inputs
 *
 * Uses vsprintf for processing formatting.
 *
 * @param[inout] log logging context
 * @param lvl logging level
 * @param ... arguments for log string format, first argument must be log string format
 * @return logged string stored in log context buffer (NULL if error)
 */
#define log_printfl(log, lvl, ...) log_printf_long(log, __FILE__, __LINE__, lvl, __VA_ARGS__)