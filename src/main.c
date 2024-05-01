/**
 * @file main.c
 * @author Rob Griffith
 */

#include "cli.h"
#include "cypher.h"
#include "fdio.h"
#include "log.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEF_BUFSZ "2056"
#define DEF_STRSZ 128
#define OUTFILE_FLAG (O_WRONLY | O_CREAT | O_TRUNC)
#define OUTFILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/**
 * @brief check stream health and print status if in bad state
 *
 * @param log logging context
 * @param bio buffered I/O context
 * @param name stream name
 * @return buffered I/O context, NULL if stream in bad state
 */
bufferedio_t *_check_stream_status(log_t *log, bufferedio_t *bio, const char *name)
{
    bufferedio_t *rv = bio;
    const int status = bio_status(bio);
    if (status <= BIO_STATUS_INIT)
    {
        char sstr[DEF_STRSZ]; /* status string buffer */
        const char *fmt = "failed to initialize %s stream: [status %d] %s\n";
        fprintf(stderr, fmt, name, status, bio_status_str(bio, sstr, sizeof(sstr)));
        log_printfl(log, LOG_ERROR, fmt, name, status, sstr);
        rv = NULL;
    }
    return rv;
}

log_t *_init_log(cli_t *cli, int bufsz, log_t *log)
{
    log_t *rv = log;
    const cli_opt_t *logopt = cli_get_opt(cli, "logfile");
    if (logopt && logopt->val)
    {
        if (bufsz < 0)
        {
            /* infinite memory, storing string */
            bio_wrap(&log->out, NULL);
        }
        else
        {
            fdio_wrap(&log->out, open(logopt->val, OUTFILE_FLAG, OUTFILE_MODE), bufsz, FDIO_CLOSE);
        }
        rv = _check_stream_status(log, &log->out, "log file") ? log : NULL;
    }
    return rv;
}

bufferedio_t *_init_key(cli_t *cli, int bufsz, log_t *log, bufferedio_t *bio)
{
    const cli_opt_t *kfopt = cli_get_opt(cli, "keyfile");
    const cli_arg_t *keyarg = cli_get_arg(cli, "key");
    if (kfopt && kfopt->val)
    {
        /* keyfile */
        log_printfl(log, LOG_INFO, "using bytes in file \"%s\" as key\n", keyarg->val);
        fdio_wrap(bio, open(keyarg->val, O_RDONLY), bufsz < 0 ? 0 : bufsz, FDIO_CLOSE);
        buffer_t buf = {0};
        if (bufsz < 0 && bio_read_all(bio, &buf))
        {
            bufferedio_t kfdb = *bio;
            bio_wrap(bio, &buf);
            bio_dfree(&kfdb);
        }
    }
    else
    {
        bio_wrap(bio, NULL);
        buf_copy(&bio->data.buf, keyarg->val, strlen(keyarg->val));
    }
    return _check_stream_status(log, bio, "key");
}

bufferedio_t *_init_input(cli_t *cli, int bufsz, log_t *log, bufferedio_t *bio)
{
    const cli_opt_t *ifopt = cli_get_opt(cli, "infile");
    int fd, cflags;
    if (ifopt && ifopt->val)
    {
        /* input file */
        log_printfl(log, LOG_INFO, "using bytes in file \"%s\" as input\n", ifopt->val);
        fd = open(ifopt->val, O_RDONLY);
        cflags = FDIO_CLOSE;
    }
    else
    {
        fd = STDIN_FILENO;
        cflags = 0;
    }
    fdio_wrap(bio, fd, bufsz < 0 ? 0 : bufsz, cflags);
    buffer_t buf = {0};
    if (bufsz < 0 && bio_read_all(bio, &buf))
    {
        bufferedio_t ifdb = *bio;
        bio_wrap(bio, &buf);
        bio_dfree(&ifdb);
    }
    return _check_stream_status(log, bio, "input");
}

bufferedio_t *_init_output(cli_t *cli, int bufsz, log_t *log, bufferedio_t *bio)
{
    const cli_opt_t *ofopt = cli_get_opt(cli, "outfile");
    int fd, cflags;
    if (bufsz < 0)
    {
        /* fully buffered */
        bio_wrap(bio, NULL);
    }
    else
    {
        if (ofopt && ofopt->val)
        {
            /* output file */
            log_printfl(log, LOG_INFO, "writing program output to file \"%s\"\n", ofopt->val);
            fd = open(ofopt->val, OUTFILE_FLAG, OUTFILE_MODE);
            cflags = FDIO_CLOSE;
        }
        else
        {
            fd = STDOUT_FILENO;
            cflags = 0;
        }
        fdio_wrap(bio, fd, bufsz < 0 ? 0 : bufsz, cflags);
    }
    return _check_stream_status(log, bio, "output");
}

int _flush_bio_buffers(cli_t *cli, log_t *log, bufferedio_t *output)
{
    int rv = 1; /* return 0 on error */
    /** write output entirely buffered in memory */
    bufferedio_t outfinal = {0};
    if (_init_output(cli, 0, log, &outfinal))
    {
        const size_t wsz = bio_write(&outfinal, output->data.buf.data, output->data.buf.size);
        if (wsz != output->data.buf.size)
        {
            const char *fmt = "only wrote %zu of %zu bytes in fully buffered output\n";
            log_printfl(log, LOG_WARNING, fmt, wsz, output->data.buf.size);
            fprintf(stderr, fmt, wsz, output->data.buf.size);
        }
    }
    else
    {
        const char *fmt = "failed to initialize final stream for writing fully buffered output\n";
        log_printfl(log, LOG_ERROR, fmt);
        fprintf(stderr, fmt);
        rv = 0;
    }
    bio_dfree(&outfinal);
    /** write log entirely buffered in memory */
    log_t logfinal = {0};
    if (_init_log(cli, 0, &logfinal))
    {
        const size_t wsz = bio_write(&logfinal.out, log->out.data.buf.data, log->out.data.buf.size);
        if (wsz != log->out.data.buf.size)
        {
            const char *fmt = "only wrote %zu of %zu bytes in fully buffered log\n";
            fprintf(stderr, fmt, wsz, log->out.data.buf.size);
        }
    }
    else
    {
        const char *fmt = "failed to initialize final stream for writing fully buffered log\n";
        fprintf(stderr, fmt);
        rv = 0;
    }
    bio_dfree(&outfinal);
    return rv;
}

int main(int argc, char **argv)
{
    /* core objects */
    int rv = 0;
    log_t log = {0};
    bufferedio_t key = {0};
    bufferedio_t input = {0};
    bufferedio_t output = {0};
    const cli_opt_t *opt;
    /* cli parsing */
    cli_arg_t args[] = {
        {"key", "key used to pseudo-encrypt input bytes", NULL}};
    cli_opt_t opts[] = {
        {'h', "help", "print application usage (to stderr)", NULL, NULL, NULL},
        {'s', "sha256", "output SHA256 hash of key (ignore input)", NULL, NULL, NULL},
        {'b', "bufsize", "set buffer size for file io in bytes", "bytes", DEF_BUFSZ, NULL},
        {'k', "keyfile", "use bytes of file at <key> argument as key", NULL, NULL, NULL},
        {'l', "logfile", "log verbose info to filepath", "path", NULL, NULL},
        {'i', "infile", "read input from filepath (instead of stdin)", "path", NULL, NULL},
        {'o', "outfile", "write output to filepath (instead of stdout)", "path", NULL, NULL}};
    cli_t cli = {
        NULL,
        sizeof(args) / sizeof(cli_arg_t),
        args,
        sizeof(opts) / sizeof(cli_opt_t),
        opts};
    if (!cli_parse(argc, argv, &cli))
    {
        cli_print_usage(&cli);
        goto error;
    }
    opt = cli_get_opt(&cli, "help");
    if (opt && opt->val)
    {
        cli_print_usage(&cli);
    }
    opt = cli_get_opt(&cli, "bufsize");
    const int bufsz = atoi(opt ? opt->val : DEF_BUFSZ);
    /* initialization (let all core objects attempt to initialize) */
    size_t init_err = 0;
    init_err += _init_log(&cli, bufsz, &log) ? 0 : 1;
    init_err += _init_key(&cli, bufsz, &log, &key) ? 0 : 1;
    init_err += _init_input(&cli, bufsz, &log, &input) ? 0 : 1;
    init_err += _init_output(&cli, bufsz, &log, &output) ? 0 : 1;
    if (init_err)
    {
        /* failed initialization */
        const char *fmt = "Initialization failed\n";
        fprintf(stderr, fmt);
        log_printfl(&log, LOG_ERROR, fmt);
        goto error;
    }
    if (bufsz == 0)
    {
        log_printfl(&log, LOG_INFO, "buffer size 0, not buffer inputs or outputs\n");
    }
    else if (bufsz < 0)
    {
        log_printfl(&log, LOG_INFO, "buffer size %d < 0, will use unlimited size\n", bufsz);
    }
    if (bio_status(&log.out) > BIO_STATUS_INIT)
    {
        /* checking log status before to save bio_status_str work */
        char sstr[DEF_STRSZ];
        const char *fmt = "%s stream: %s\n";
        log_printfl(&log, LOG_INFO, fmt, "log", bio_status_str(&log.out, sstr, sizeof(sstr)));
        log_printfl(&log, LOG_INFO, fmt, "key", bio_status_str(&key, sstr, sizeof(sstr)));
        log_printfl(&log, LOG_INFO, fmt, "input", bio_status_str(&input, sstr, sizeof(sstr)));
        log_printfl(&log, LOG_INFO, fmt, "output", bio_status_str(&output, sstr, sizeof(sstr)));
    }
    /* main work */
    sha256hash_t key_hash;
    sha256(&key, &key_hash);
    opt = cli_get_opt(&cli, "sha256");
    if (bio_status(&log.out) > BIO_STATUS_INIT || (opt && opt->val))
    {
        sha256hex_t hash_str;
        sha256_hexstr(&key_hash, &hash_str);
        log_printfl(&log, LOG_INFO, "SHA256 key hash: 0x%s\n", hash_str.str);
        if (opt && opt->val)
        {
            bio_write(&output, hash_str.str, strlen(hash_str.str));
            /* no more work, output is only hash */
            goto end;
        }
    }
    const size_t csz = cypher_xor(&input, &key_hash, &output);
    log_printfl(&log, LOG_INFO, "encoded %zu bytes\n", csz);
    if (bufsz < 0 && !_flush_bio_buffers(&cli, &log, &output))
    {
        goto error;
    }
    goto end;
error:
    rv = 1;
end:
    bio_dfree(&output);
    bio_dfree(&input);
    bio_dfree(&key);
    bio_dfree(&log.out);
    return rv;
}