/**
 * @file main.c
 * @author Rob Griffith
 */

#include "cypher.h"
#include "fileio.h"

#include <stdio.h>
#include <string.h>

void print_usage(const char *oname)
{
    printf("\
usage:\n\
    %s <filepath> <key>\n\
\n",
           oname);
}

int main(int argc, char **argv)
{
    int rv = 1;
    if (argc != 3)
    {
        print_usage(argv[0]);
        return rv;
    }
    const char *path_in = argv[1];
    printf("filepath: %s\n", path_in);
    buffer_h buf = read_file(path_in);
    if (!buf)
    {
        return rv;
    }
    const char *key = argv[2];
    printf("key: %s\n", key);
    if (!cypher(buf, key))
    {
        return rv;
    }
    const char *suffix = ".enc";
    buffer_h strbuf = buf_concat(path_in, strlen(path_in), suffix, strlen(suffix) + 1);
    const char *path_out = strbuf ? buf_data(strbuf) : "cypher.enc";
    const size_t wsz = write_file(path_out, buf);
    if (wsz == buf_size(buf))
    {
        printf("wrote encoded output to file %s\n", path_out);
        rv = 0;
    }
    buf_free(strbuf);
    buf_free(buf);
    return rv;
}