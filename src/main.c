/**
 * @file main.c
 * @author Rob Griffith
 */

#include "cli.h"
#include "cypher.h"
#include "fdio.h"
#include "fileio.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOPTS 4
#define NARGS 2

#define OUTFILE_FLAG (O_WRONLY | O_CREAT | O_TRUNC)
#define OUTFILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

int main(int argc, char **argv)
{
    /* core objects */
    int rv = 0;
    bufferedio_t infile = {0};
    bufferedio_t outfile = {0};
    bufferedio_t key = {0};
    /* cli parsing */
    option_t opts[NOPTS] = {
        {'h', "help", "print application usage", FLAG, NULL, NULL},
        {'b', "bufsize", "set buffer size for file io", PARAM, "1028", NULL},
        {'f', "filekey", "read key from file (key argument is path)", FLAG, NULL, NULL},
        {'o', "outpath", "output filepath", PARAM, "out.enc", NULL}};
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
        buffer_t filebuf = {0};
        fio_read_all(&filebuf, cli.args[0].val);
        bio_wrap(&infile, &filebuf);
        bio_wrap(&outfile, NULL);
        if (cli.opts[2].val)
        {
            fio_read_all(&filebuf, cli.args[1].val);
            bio_wrap(&key, &filebuf);
        }
    }
    else
    {
        /* restricted buffered io */
        fdio_wrap(&infile, open(cli.args[0].val, O_RDONLY), bufsz);
        fdio_wrap(&outfile, open(cli.opts[3].val, OUTFILE_FLAG, OUTFILE_MODE), bufsz);
        if (cli.opts[2].val)
        {
            fdio_wrap(&key, open(cli.args[1].val, O_RDONLY), bufsz);
        }
    }
    if (!cli.opts[2].val)
    {
        buffer_t keybuf = {0};
        buf_copy(&keybuf, cli.args[1].val, strlen(cli.args[1].val));
        bio_wrap(&key, &keybuf);
    }
    /* check initialization */
    if (!(bio_status(&infile) > BIO_STATUS_INIT && bio_status(&outfile) > BIO_STATUS_INIT && bio_status(&key) > BIO_STATUS_INIT))
    {
        fprintf(stderr, "initialization failed\n");
        goto error;
    }
    /* work */
    printf("working...\n");
    rv = cypher(&infile, &key, &outfile);
    if (bufsz <= 0 && fio_write_all(cli.opts[3].val, &outfile.data.buf) != outfile.data.buf.size)
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