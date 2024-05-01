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

/**
 * @todo client-server applications for networked messaging
 * (better encrytpion will be built on top of this, initially making-do with XOR)
 * (should be careful w/ bufferedio, make sure to flush writes and only read when and what is needed)
 * should build local server application that always listens on specified port and stores messages and connection configs
 * will have support for multiple named connections associated with peer IPs and ports
 * server will have connection config file for persistence between client sessions
 * messaging client will "subscribe" to updates from server
 *  e.g. updates to connection statuses, new messages, delivery & read-receipts, etc
 * messaging client will have multiple stdin cli structs for commands
 *  0. help
 *      print usage of all commands (all cli structs)
 *  1. exit
 *      exits application
 *  2. status
 *      show status of connection to peer server (and list addr)
 *      for each connection
 *      show connection up/down status next to name (ping peer server for response, show last connection timestamp)
 *      if non-zero show number of own unsent messages waiting for peer server to be responsive
 *      if non-zero show number of sent but unread messages (no read-receipt from peer client & server)
 *      if non-zero show number of unread messages for user to read
 *  3. conn <name> [-c <addr>] [-d]
 *      -c create named connection for peer (server stores into config)
 *      -d delete named connection
 *      switch to connection (if -d not specified, and if connection exists)
 *      show specific connection status upon switching
 *  4. read [-n <#>] [-t <timestamp>]
 *      -n print last # messages
 *      -t print messages at and after timestamp
 *      prints all unread messages to stdout (options specify other behavior)
 *  5. send <message>
 *      send message to server to forward to connection
 */

int main(int argc, char **argv)
{
    /** core objects */
    int rv = 0;
    bufferedio_t infile = {0};
    /* cli parsing */
    cli_arg_t args[] = {
        {"inpath", "input filepath", NULL},
        {"key", "key used to encrypt file data", NULL}};
    cli_opt_t opts[] = {
        {'h', "help", "print application usage", NULL, NULL, NULL},
        {'v', "verbose", "more verbose logging", NULL, NULL, NULL},
        {'b', "bufsize", "set buffer size for file io in bytes", "bytes", "1028", NULL},
        {'k', "keyfile", "treat <key> argument as file to read key from", NULL, NULL, NULL},
        {'o', "outpath", "output filepath", "path", "out.enc", NULL}};
    cli_t cli = {
        NULL,
        sizeof(args) / sizeof(cli_arg_t),
        args,
        sizeof(opts) / sizeof(cli_opt_t),
        opts};

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