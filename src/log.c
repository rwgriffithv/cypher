/**
 * @file log.c
 * @author Rob Griffith
 */

#include "log.h"
#include "bstring.h"

#include <stdarg.h>
#include <string.h>
#include <time.h>

const char *log_lvlstr(log_lvl_t lvl)
{
    const char *rv;
    switch (lvl)
    {
    case LOG_INFO:
        rv = "INFO";
        break;
    case LOG_WARNING:
        rv = "WARNING";
        break;
    case LOG_ERROR:
        rv = "ERROR";
        break;
    default:
        rv = "UNKNOWN";
    }
    return rv;
}

const char *log_printf(log_t *log, const char *fmt, ...)
{
    const char *str = NULL;
    if (bio_status(&log->out) <= BIO_STATUS_INIT)
    {
        goto end;
    }
    va_list args;
    va_start(args, fmt);
    buf_clear(&log->buf);
    str = bstr_printf(&log->buf, fmt, args);
    va_end(args);
    if (str)
    {
        bio_write(&log->out, str, log->buf.size);
    }
end:
    return str;
}

const char *log_printf_long(log_t *log, const char *file, int line, log_lvl_t lvl, const char *fmt, ...)
{
    const char *str = NULL;
    if (bio_status(&log->out) <= BIO_STATUS_INIT)
    {
        goto end;
    }
    buf_clear(&log->buf);
    char tstr[32]; /* known max length of string */
    time_t t;
    time(&t);
    const char *timestr;
    if (ctime_r(&t, tstr))
    {
        tstr[strlen(tstr) - 1] = '\0'; /* overwrite newline */
        timestr = tstr;
    }
    else
    {
        timestr = "TIME ERROR";
    }
    const char *lvlstr = log_lvlstr(lvl);
    va_list args;
    va_start(args, fmt);
    const int valid = bstr_printf(&log->buf, "[%s] [%s:%d] [%s] ", timestr, file, line, lvlstr) && bstr_vprintf(&log->buf, fmt, args);
    va_end(args);
    if (!valid)
    {
        goto end;
    }
    str = (const char *)log->buf.data;
    bio_write(&log->out, str, log->buf.size - 1); /* exclude null byte */
end:
    return str;
}