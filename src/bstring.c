/**
 * @file bstring.c
 * @author Rob Griffith
 */

#include "bstring.h"

#include <stdio.h>
#include <string.h>

void _trim_null(buffer_t *buf)
{
    if (buf->size && ((const char *)buf->data)[buf->size - 1] == '\0')
    {
        buf_resize(buf, buf->size - 1);
    }
}

const char *bstr_concat(buffer_t *buf, const char *str)
{
    _trim_null(buf);
    /* include null byte at end */
    return buf_push(buf, str, strlen(str) + 1) ? (const char *)buf->data : NULL;
}

const char *bstr_printf(buffer_t *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const char *rv = bstr_vprintf(buf, fmt, args);
    va_end(args);
    return rv;
    /*
    const char *rv = NULL;
    if (!(buf->capacity || buf_init(buf, 64)))
    {
        goto end;
    }
    _trim_null(buf);
    const size_t insz = buf->size;
    char *s = (char *)buf->data + insz;
    size_t n = buf->capacity - insz;
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(s, n, fmt, args);
    va_end(args);
    if (len >= 0 && len >= n)
    {
        n = (size_t)len + 1;
        if (!buf_resize(buf, insz + n))
        {
            goto end;
        }
        s = (char *)buf->data + insz;
        va_start(args, fmt);
        len = vsnprintf(s, n, fmt, args);
        va_end(args);
    }
    if (len < 0)
    {
        goto end;
    }
    buf_resize(buf, insz + (size_t)len + 1);
    rv = s;
end:
    return rv;
    */
}

const char *bstr_vprintf(buffer_t *buf, const char *fmt, va_list args)
{
    const char *rv = NULL;
    if (!(buf->capacity || buf_init(buf, 64)))
    {
        goto end;
    }
    _trim_null(buf);
    const size_t insz = buf->size;
    char *s = (char *)buf->data + insz;
    size_t n = buf->capacity - insz;
    va_list args_cpy;
    va_copy(args_cpy, args);
    int len = vsnprintf(s, n, fmt, args);
    va_end(args);
    if (len >= 0 && len >= n)
    {
        n = (size_t)len + 1;
        if (!buf_resize(buf, insz + n))
        {
            goto end;
        }
        s = (char *)buf->data + insz;
        len = vsnprintf(s, n, fmt, args_cpy);
    }
    va_end(args_cpy);
    if (len < 0)
    {
        goto end;
    }
    buf_resize(buf, insz + (size_t)len + 1); /* include null byte */
    rv = s;
end:
    return rv;
}