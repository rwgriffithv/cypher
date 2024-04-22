/**
 * @file tokenize.c
 * @author Rob Griffith
 */

#include "tokenize.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>

const char *tkz_parse_str_token(bufferedio_t *bio, buffer_t *buf, char *end)
{
    int err = 0;
    int quoted = 0;
    int escaped = 0;
    size_t nc = 0;
    char c;
    do
    {
        if (!bio_read(bio, &c, sizeof(char)))
        {
            /* check error for EOF or EINTR */
            const int status = bio_status(bio);
            printf("bio_read returned 0, status: %d\n", status);
            if (status <= BIO_STATUS_INIT)
            {
                printf("processing bio_read 0 as error\n");
                int retry = 0;
                switch (BIO_STATUS_INIT - status)
                {
                case EINTR:
                case EAGAIN:
                    /* retry (TODO: maybe add delay) */
                    retry = 1;
                    break;
                default:
                    /* stop with error */
                    err = 1;
                }
                if (retry)
                {
                    continue;
                }
            }
            else
            {
                /* EOF, stop cleanly */
                printf("processing bio_read 0 as EOF\n");
                c = EOF;
            }
            /* stop processing */
            break;
        }
        if (escaped)
        {
            escaped = 0;
            /* escaped whitespace universally allowed */
            if (!isspace(c))
            {
                switch (c)
                {
                case 'a':
                    c = '\a';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'e':
                    c = '\e';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'v':
                    c = '\v';
                    break;
                case '\\':
                case '\'':
                case '"':
                case '?':
                    /* escaped to be literal, leave unchanged */
                    break;
                default:
                    /* unrecognized escaped character, push full escape sequence */
                    nc += buf_push(buf, "\\", sizeof(c));
                }
            }
            nc += buf_push(buf, &c, sizeof(c));
        }
        else if (c == '\\')
        {
            escaped = 1;
        }
        else if (quoted)
        {
            if (c == '"')
            {
                quoted = 0;
            }
            else
            {
                nc += buf_push(buf, &c, sizeof(c));
            }
        }
        else if (c == '"')
        {
            quoted = 1;
        }
        else if (isspace(c))
        {
            if (nc)
            {
                break;
            }
        }
        else
        {
            nc += buf_push(buf, &c, sizeof(c));
        }
    } while (1);
    const char *rv = NULL;
    if (!err)
    {
        buf_push(buf, "", sizeof(c));
        rv = (const char *)buf->data + buf->size - nc - 1;
        if (end)
        {
            *end = c;
        }
    }
    return rv;
}