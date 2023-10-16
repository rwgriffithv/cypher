/**
 * @file main.c
 * @author Rob Griffith
 */

#include "cli.h"
#include "cypher.h"
#include "fileio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOPTS 4
#define NARGS 2

int main(int argc, char **argv)
{
    /* core objects */
    int rv = 0;
    bufferedio_t infile = {0};
    bufferedio_t outfile = {0};
    bufferedio_t key = {0};
    /* cli parsing */
    option_t opts[NOPTS] = {
        {'h', "help", "print application usage", FLAG, NULL},
        {'b', "bufsize", "set buffer size for file io", PARAM, "1028"},
        {'f', "filekey", "read key from file (key argument is path)", FLAG, NULL},
        {'o', "outpath", "output filepath", PARAM, "out.enc"}};
    argument_t args[NARGS] = {
        {"inpath", "input filepath", NULL},
        {"key", "key used to encrypt file data", NULL}};
    cli_t cli = {
        NOPTS,
        opts,
        NARGS,
        args};
    if (cli_parse(argc, argv, &cli) < 0)
    {
        print_usage(argv[0], &cli);
        goto error;
    }
    if (cli.opts[0].val)
    {
        print_usage(argv[0], &cli);
    }
    const int bufsz = atoi(cli.opts[1].val);
    /* initialization */
    if (bufsz <= 0)
    {
        /* unlimited buffer, all in memory */
        printf("buffer size %d <= 0, will use unlimited size\n", bufsz);
        bio_wrap(&infile, fio_read_all(cli.args[0].val));
        bio_wrap(&outfile, buf_init(0));
        if (cli.opts[2].val)
        {
            bio_wrap(&key, fio_read_all(cli.args[1].val));
        }
    }
    else
    {
        /* restricted buffered io */
        fio_init(&infile, bufsz) && fio_open(&infile, cli.args[0].val, "r");
        fio_init(&outfile, bufsz) && fio_open(&outfile, cli.opts[3].val, "w");
        if (cli.opts[2].val)
        {
            fio_init(&key, bufsz) && fio_open(&key, cli.args[1].val, "r");
        }
    }
    if (!cli.opts[2].val)
    {
        bio_wrap(&key, buf_copy(cli.args[1].val, strlen(cli.args[1].val)));
    }
    /* check initialization */
    if (!(bio_status(&infile) > BIO_STATUS_INIT && bio_status(&outfile) > BIO_STATUS_INIT && bio_status(&key) > BIO_STATUS_INIT))
    {
        fprintf(stderr, "initialization failed\n");
        goto error;
    }
    /* work */
    rv = cypher(&infile, &key, &outfile);
    if (bufsz <= 0 && fio_write_all(cli.opts[3].val, outfile.data.buf) != buf_size(outfile.data.buf))
    {
        goto error;
    }
    goto end;
error:
    rv = 1;
end:
    bio_dfree(&infile);
    bio_dfree(&key);
    bio_dfree(&outfile);
    return rv;
}