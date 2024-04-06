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

#define OUTFILE_FLAG (O_WRONLY | O_CREAT | O_TRUNC)
#define OUTFILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

int main(int argc, char **argv)
{
    /* core objects */
    buffer_t tempbuf = {0};
    int rv = 0;
    bufferedio_t infile = {0};
    bufferedio_t outfile = {0};
    bufferedio_t key = {0};
    /* cli parsing */
    cli_opt_t opts[] = {
        {'h', "help", "print application usage", NULL, NULL, NULL},
        {'b', "bufsize", "set buffer size for file io in bytes", "bytes", "1028", NULL},
        {'k', "keyfile", "treat <key> argument as file to read key from", NULL, NULL, NULL},
        {'o', "outpath", "output filepath", "path", "out.enc", NULL}};
    cli_arg_t args[] = {
        {"inpath", "input filepath", NULL},
        {"key", "key used to encrypt file data", NULL}};
    cli_t cli = {
        sizeof(opts) / sizeof(cli_opt_t),
        opts,
        sizeof(args) / sizeof(cli_arg_t),
        args};
    if (!cli_parse(argc, argv, &cli))
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
        fio_read_all(&tempbuf, cli.args[0].val);
        bio_wrap(&infile, &tempbuf);
        bio_wrap(&outfile, NULL);
        if (cli.opts[2].val)
        {
            fio_read_all(&tempbuf, cli.args[1].val);
            bio_wrap(&key, &tempbuf);
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
        buf_copy(&tempbuf, cli.args[1].val, strlen(cli.args[1].val));
        bio_wrap(&key, &tempbuf);
    }
    /* check initialization */
    if (bio_status(&infile) <= BIO_STATUS_INIT)
    {
        fprintf(stderr, "failed to initialize input file stream\n");
        goto error;
    }
    if (bio_status(&outfile) <= BIO_STATUS_INIT)
    {
        fprintf(stderr, "failed to initialize output file stream\n");
        goto error;
    }
    if (bio_status(&key) <= BIO_STATUS_INIT)
    {
        fprintf(stderr, "failed to initialize key input stream\n");
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