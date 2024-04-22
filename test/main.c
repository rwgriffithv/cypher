/**
 * @file main.c
 * @author Rob Griffith
 */

#include "cli.h"
#include "fdio.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
TODO: new copy of main function to test stdin/stdout default behavior
    (flexing usage of fdio)
    handle input from stdin and put output to stdout
    key still specified as required arg, using -k flag for file
    bufsize remains the same
    stdin will be default input without -i flag
    stdout will be default output without -o flag
    log file option -l flag will replace -v option
*/
/*
TODO: new application for p2p sending and receiving cyphered messages and logging them to file
    server implementation to handle multiple connections
    stdin cli options to switch between connections
*/

int main(int argc, char **argv)
{
    /** core objects */
    int rv = 0;
    bufferedio_t infile = {0};
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
        NULL,
        sizeof(opts) / sizeof(cli_opt_t),
        opts,
        sizeof(args) / sizeof(cli_arg_t),
        args};

    /** test parsing lines in file passes as first argument */
    fdio_wrap(&infile, open(argv[1], O_RDONLY), 32);
    buffer_t linebuf = {0};
    size_t maxcnt = 10;
    size_t icnt = 0;
    do
    {
        cli_t cli_temp = {
            NULL,
            sizeof(opts) / sizeof(cli_opt_t),
            opts,
            sizeof(args) / sizeof(cli_arg_t),
            args};
        buf_clear(&linebuf);
        const int clirv = cli_parse_line(&infile, &linebuf, &cli_temp);
        size_t lbi = 0;
        printf("\n\nline values:\n");
        while (lbi < linebuf.size)
        {
            char *str = (char *)linebuf.data + lbi;
            printf("%s\n", str);
            lbi += strlen(str) + 1;
        }
        printf("\ncommand: %s\n", cli_temp.cmd);
        printf("\nparsed args:\n");
        size_t ip;
        for (ip = 0; ip < cli_temp.nargs; ip++)
        {
            cli_arg_t *arg = cli_temp.args + ip;
            printf("%s: %s\n", arg->name, arg->val ? arg->val : "NOTFOUND");
        }
        printf("\nparsed opts:\n");
        for (ip = 0; ip < cli_temp.nopts; ip++)
        {
            cli_opt_t *opt = cli_temp.opts + ip;
            printf("%s: %s\n", opt->name_l, opt->val ? opt->val : "NOTFOUND");
        }
        if (clirv == 0)
        {
            printf("\n\n cli_parse_line failed gracefully, EOF\n");
            break;
        }
        else if (clirv < 0)
        {
            char sstr[128];
            const int status = bio_status(&infile);
            printf("\n\n cli_parse_line returned error, infile status: [%d] %s\n", status, bio_strstatus(&infile, sstr, sizeof(sstr)));
            if (status <= BIO_STATUS_INIT)
            {
                printf("bad exit due to error\n");
                break;
            }
            else
            {
                printf("buffered input still good, parsing error\n");
            }
        }
        icnt++;
    } while (icnt < maxcnt);
    bio_dfree(&infile);
    return 0;
}