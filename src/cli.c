/**
 * @file cli.c
 * @author Rob Griffith
 */

#include "cli.h"
#include "tokenize.h"

#include <stdio.h>
#include <string.h>

void _print_args(const cli_t *cli)
{
    int i;
    if (cli->nargs <= 0)
    {
        return;
    }
    fprintf(stderr, "\n\
arguments:");
    for (i = 0; i < cli->nargs; i++)
    {
        fprintf(stderr, "\n\
    %s\n\
        %s",
                cli->args[i].name, cli->args[i].desc);
    }
}

void _print_opts(const cli_t *cli)
{
    const cli_opt_t *opt;
    size_t i;
    if (cli->nopts <= 0)
    {
        return;
    }
    fprintf(stderr, "\n\
options:");
    for (i = 0; i < cli->nopts; i++)
    {
        opt = cli->opts + i;
        fprintf(stderr, "\n\
    --%s",
                opt->name_l);
        if (opt->param)
        {
            fprintf(stderr, " <%s>", opt->param);
        }
        if (opt->name_s != '\0')
        {
            fprintf(stderr, "\n\
        -%c",
                    opt->name_s);
            if (opt->param)
            {
                fprintf(stderr, " <%s>", opt->param);
            }
        }
        if (opt->def)
        {
            fprintf(stderr, "\n\
        (default: %s)",
                    opt->def);
        }
        fprintf(stderr, "\n\
        %s",
                opt->desc);
    }
}

cli_opt_t *_find_opt_l(cli_t *cli, const char *name_l)
{
    cli_opt_t *rv = cli_get_opt(cli, name_l);
    if (!rv)
    {
        fprintf(stderr, "invalid unknown option --%s\n", name_l);
    }
    return rv;
}

cli_opt_t *_find_opt_s(cli_t *cli, const char name_s)
{
    cli_opt_t *rv = NULL;
    size_t i;
    for (i = 0; i < cli->nopts; i++)
    {
        if (name_s == cli->opts[i].name_s)
        {
            rv = cli->opts + i;
            break;
        }
    }
    if (!rv)
    {
        fprintf(stderr, "invalid unknown short option -%c\n", name_s);
    }
    return rv;
}

const cli_opt_t *_opt_parse(size_t argc, char *const *argv, cli_opt_t *opt, size_t *pidx)
{
    if (!opt)
    {
        return NULL;
    }
    if (opt->param)
    {
        if (*pidx >= argc)
        {
            /* missing required value for option */
            fprintf(stderr, "missing required argument <%s> for option --%s\n", opt->param, opt->name_l);
            return NULL;
        }
        opt->val = argv[*pidx];
        *pidx = *pidx + 1;
    }
    else
    {
        opt->val = "true";
    }
    return opt;
}

void cli_print_usage(const cli_t *cli)
{
    fprintf(stderr, "\
usage:\n\
    %s",
            cli->cmd);
    int i;
    for (i = 0; i < cli->nargs; i++)
    {
        fprintf(stderr, " <%s>", cli->args[i].name);
    }
    if (cli->nopts > 0)
    {
        fprintf(stderr, " [options]");
    }
    _print_args(cli);
    _print_opts(cli);
    fprintf(stderr, "\n");
}

cli_arg_t *cli_get_arg(cli_t *cli, const char *name)
{
    cli_arg_t *rv = NULL;
    size_t i;
    for (i = 0; i < cli->nargs; i++)
    {
        if (strcmp(name, cli->args[i].name) == 0)
        {
            rv = cli->args + i;
            break;
        }
    }
    return rv;
}

cli_opt_t *cli_get_opt(cli_t *cli, const char *name_l)
{
    cli_opt_t *rv = NULL;
    size_t i;
    for (i = 0; i < cli->nopts; i++)
    {
        if (strcmp(name_l, cli->opts[i].name_l) == 0)
        {
            rv = cli->opts + i;
            break;
        }
    }
    return rv;
}

const cli_t *cli_parse(size_t argc, char *const *argv, cli_t *cli)
{
    size_t i;
    /* default options */
    for (i = 0; i < cli->nopts; i++)
    {
        cli_opt_t *opt = cli->opts + i;
        if (opt->param)
        {
            opt->val = opt->def;
        }
        else
        {
            // flag, no defaults allowed
            opt->def = NULL;
            opt->val = NULL;
        }
    }
    /* parse binary as command name */
    if (argc)
    {
        const char *argv0 = argv[0];
        cli->cmd = argv0;
        while (*argv0 != '\0')
        {
            if (*argv0 == '/')
            {
                cli->cmd = argv0 + 1;
            }
            argv0++;
        }
    }
    /* parse cli values, skipping binary name */
    size_t i_arg = 0;
    i = 1;
    while (i < argc)
    {
        if (argv[i][0] == '-' && argv[i][1] != '\0')
        {
            /* parse option */
            cli_opt_t *opt = NULL;
            size_t pidx = i + 1;
            if (argv[i][1] == '-')
            {
                /* parse full option */
                opt = _find_opt_l(cli, argv[i] + 2);
                if (!_opt_parse(argc, argv, opt, &pidx))
                {
                    return NULL;
                }
            }
            else
            {
                /* parse short option(s) */
                size_t j = 1;
                while (argv[i][j] != '\0')
                {
                    opt = _find_opt_s(cli, argv[i][j++]);
                    if (!_opt_parse(argc, argv, opt, &pidx))
                    {
                        return NULL;
                    }
                }
            }
            i = pidx;
        }
        else if (i_arg < cli->nargs)
        {
            /* parse argument */
            cli->args[i_arg++].val = argv[i++];
        }
        else
        {
            /* invalid extra argument */
            fprintf(stderr, "invalid extra argument \"%s\" (exactly %zu arguments required)\n", argv[i], cli->nargs);
            return NULL;
        }
    }
    if (i_arg < cli->nargs)
    {
        /* missing required arguments */
        fprintf(stderr, "only parsed %zu of required %zu arguments\n", i_arg, cli->nargs);
        return NULL;
    }
    return cli;
}

int cli_parse_line(bufferedio_t *bio, buffer_t *buf, cli_t *cli)
{
    char end = 0;
    if (!cli->cmd)
    {
        /* cmd token not parsed already, parse from line */
        cli->cmd = tkz_parse_str_token(bio, buf, &end);
    }
    buffer_t argvbuf = {0}; /* store first as relative offsets to buf->data */
    size_t offset;
    if (cli->cmd)
    {
        /* cli->cmd always expected to be parsed into same buffer buf */
        offset = cli->cmd - (const char *)buf->data;
        buf_push(&argvbuf, &offset, sizeof(offset));
    }
    while (!(end == '\n' || end == EOF))
    {
        const char *tok = tkz_parse_str_token(bio, buf, &end);
        if (!tok || *tok == '\0')
        {
            break;
        }
        offset = tok - (const char *)buf->data;
        buf_push(&argvbuf, &offset, sizeof(offset));
    }
    size_t argc = argvbuf.size / sizeof(char *);
    char **argv = argvbuf.data;
    size_t i = 0;
    for (i = 0; i < argc; i++)
    {
        argv[i] += (size_t)buf->data;
    }
    int rv;
    if (argc)
    {
        rv = cli_parse(argc, argv, cli) ? 1 : -1;
    }
    else
    {
        rv = end == EOF ? 0 : -1;
    }
    buf_free(&argvbuf);
    return rv;
}