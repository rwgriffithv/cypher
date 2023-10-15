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
        return 1;
    }
    if (cli.opts[0].val)
    {
        print_usage(argv[0], &cli);
    }
    int rv = 0;
    bufferedio_t infile;
    bufferedio_t key;
    bufferedio_t outfile;
    buffer_t *b;
    /* buffer size */
    int bufsz = atoi(cli.opts[1].val);
    /* initialization */
    if (bufsz <= 0)
    {
        /* all in memory */
        printf("buffer size %d <= 0, will use unlimited size\n", bufsz);
        b = fio_read_all(cli.args[0].val);
        rv = b ? rv : 1;
        bio_wrap(&infile, b);
        if (cli.opts[2].val)
        {
            b = fio_read_all(cli.args[1].val);
            rv = b ? rv : 1;
            bio_wrap(&key, b);
        }
        b = buf_init(0);
        rv = b ? rv : 1;
        bio_wrap(&outfile, b);
    }
    else
    {
        rv = fio_init(&infile, bufsz) || fio_open(&infile, cli.args[0].val, "r") ? 1 : rv;
        rv = cli.opts[2].val && (fio_init(&key, bufsz) || fio_open(&key, cli.args[1].val, "r")) ? 1 : rv;
        rv = fio_init(&outfile, bufsz) || fio_open(&outfile, cli.opts[3].val, "w") ? 1 : rv;
    }
    if (!cli.opts[2].val)
    {
        b = buf_copy(cli.args[1].val, strlen(cli.args[1].val));
        rv = b ? rv : 1;
        bio_wrap(&key, b);
    }
    if (!rv)
    {
        /* work */
        rv = cypher(&infile, &key, &outfile);
        if (bufsz <= 0)
        {
            rv = fio_write_all(cli.opts[3].val, outfile.data.buf) == buf_size(outfile.data.buf) ? rv : 1;
        }
    }
    fio_free(&infile);
    fio_free(&key);
    fio_free(&outfile);
    return rv;
}