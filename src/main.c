/**
 * @file main.c
 * @author Rob Griffith
 */

#include "cypher.h"
#include "fileio.h"

#include <stdio.h>
#include <string.h>

void print_usage(char *oname)
{
    printf("\
usage:\n\
    %s <mode> <path in> <seed>\n\
\n\
modes:\n\
    -e  encode\n\
    -d  decode\n\
\n",
           oname);
}

int main(int argc, char **argv)
{
    int rv = 1;
    if (argc != 4)
    {
        print_usage(argv[0]);
        return rv;
    }
    const char *path_in = argv[2];
    buffer_h buf_in = read_file(path_in);
    if (!buf_in)
    {
        return rv;
    }
    const char *mode = argv[1];
    const char *key = argv[3];
    buffer_h buf_out;
    const char *suffix;
    if (strcmp("-d", mode) == 0)
    {
        out = decode(buf_in, key);
        suffix = ".dec";
    }
    else if (strcmp("-e", mode) == 0)
    {
        out = encode(buf_in, key);
        suffix = ".enc";
    }
    else
    {
        fprintf(stderr, "invalid mode %s\n", mode);
        print_usage(argv[0]);
        buf_free(buf_in);
        return rv;
    }
    buf_free(buf_in);
    buffer_h strbuf = buf_concat(path_in, strlen(path_in), suffix, strlen(suffix) + 1);
    const char *path_out = (char *)buf_data(strbuf);
    const size_t wsz = write_file(path_out, buf_out);
    if (wsz == buf_size(buf_out))
    {
        printf("wrote full encoded output to file %s\n", path_out);
        rv = 0;
    }
    buf_free(strbuf);
    buf_free(buf_out);
    return rv;
}