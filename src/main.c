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
    int rv = 0;
    bufferedio_t infile = {0};
    bufferedio_t outfile = {0};
    bufferedio_t key = {0};
    /* cli parsing */
    cli_opt_t opts[] = {
        {'h', "help", "print application usage", NULL, NULL, NULL},
        {'v', "verbose", "more verbose logging", NULL, NULL, NULL},
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
    const int bufsz = atoi(cli.opts[2].val);
    /* initialization */
    int read_all_rv_in = 0;
    int read_all_rv_key = 0;
    if (bufsz <= 0)
    {
        /* unlimited buffer, all in memory */
        if (cli.opts[1].val)
        {
            printf("buffer size %d <= 0, will use unlimited size\n", bufsz);
        }
        bio_wrap(&infile, NULL);
        read_all_rv_in = fio_read_all(&infile.data.buf, cli.args[0].val);
        bio_wrap(&outfile, NULL);
        if (cli.opts[3].val)
        {
            bio_wrap(&key, NULL);
            read_all_rv_key = fio_read_all(&key.data.buf, cli.args[1].val);
        }
    }
    else
    {
        /* restricted buffered io */
        fdio_wrap(&infile, open(cli.args[0].val, O_RDONLY), bufsz);
        fdio_wrap(&outfile, open(cli.opts[4].val, OUTFILE_FLAG, OUTFILE_MODE), bufsz);
        if (cli.opts[3].val)
        {
            fdio_wrap(&key, open(cli.args[1].val, O_RDONLY), bufsz);
        }
    }
    if (!cli.opts[3].val)
    {
        bio_wrap(&key, NULL);
        buf_copy(&key.data.buf, cli.args[1].val, strlen(cli.args[1].val));
    }
    /* check initialization */
    int init_err = 0;
    char sstr[128];
    if (bio_status(&infile) <= BIO_STATUS_INIT || read_all_rv_in < 0)
    {
        fprintf(stderr, "failed to initialize input file stream: [status %d] %s\n", bio_status(&infile), bio_strstatus(&infile, sstr, sizeof(sstr)));
        init_err = 1;
    }
    if (bio_status(&outfile) <= BIO_STATUS_INIT)
    {
        fprintf(stderr, "failed to initialize output file stream: [status %d] %s\n", bio_status(&outfile), bio_strstatus(&outfile, sstr, sizeof(sstr)));
        init_err = 1;
    }
    if (bio_status(&key) <= BIO_STATUS_INIT || read_all_rv_key < 0)
    {
        fprintf(stderr, "failed to initialize key input stream: [status %d] %s\n", bio_status(&key), bio_strstatus(&key, sstr, sizeof(sstr)));
        init_err = 1;
    }
    if (init_err)
    {
        goto error;
    }
    if (cli.opts[1].val)
    {
        printf("input file stream: %s\n", bio_strstatus(&infile, sstr, sizeof(sstr)));
        printf("output file stream: %s\n", bio_strstatus(&outfile, sstr, sizeof(sstr)));
        printf("key file stream: %s\n", bio_strstatus(&key, sstr, sizeof(sstr)));
        printf("working...\n");
    }
    /* work */
    rv = cypher(&infile, &key, &outfile);
    if (bufsz <= 0 && fio_write_all(cli.opts[4].val, &outfile.data.buf) != outfile.data.buf.size)
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