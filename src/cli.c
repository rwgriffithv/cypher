/**
 * @file cli.c
 * @author Rob Griffith
 */

#include "cli.h"

#include <stdio.h>
#include <string.h>

cli_opt_t *find_opt_l(const char *name_l, cli_t *cli)
{
    size_t i;
    for (i = 0; i < cli->nopts; i++)
    {
        if (strcmp(name_l, cli->opts[i].name_l) == 0)
        {
            return cli->opts + i;
        }
    }
    fprintf(stderr, "invalid unknown option --%s\n", name_l);
    return NULL;
}

cli_opt_t *find_opt_s(const char name_s, cli_t *cli)
{
    size_t i;
    for (i = 0; i < cli->nopts; i++)
    {
        if (name_s == cli->opts[i].name_s)
        {
            return cli->opts + i;
        }
    }
    fprintf(stderr, "invalid unknown short option -%c\n", name_s);
    return NULL;
}

void print_args(const cli_t *cli)
{
    int i;
    if (cli->nargs <= 0)
    {
        return;
    }
    printf("\n\
arguments:");
    for (i = 0; i < cli->nargs; i++)
    {
        printf("\n\
    %s\n\
        %s",
               cli->args[i].name, cli->args[i].desc);
    }
}

void print_opts(const cli_t *cli)
{
    const cli_opt_t *opt;
    size_t i;
    if (cli->nopts <= 0)
    {
        return;
    }
    printf("\n\
options:");
    for (i = 0; i < cli->nopts; i++)
    {
        opt = cli->opts + i;
        printf("\n\
    --%s",
               opt->name_l);
        if (opt->param)
        {
            printf(" <%s>", opt->param);
        }
        if (opt->name_s != '\0')
        {
            printf("\n\
        -%c",
                   opt->name_s);
            if (opt->param)
            {
                printf(" <%s>", opt->param);
            }
        }
        if (opt->def)
        {
            printf("\n\
        (default: %s)",
                   opt->def);
        }
        printf("\n\
        %s",
               opt->desc);
    }
}

void print_usage(const char *argv0, const cli_t *cli)
{
    const char *bin = argv0;
    size_t i;
    while (*argv0 != '\0')
    {
        if (*argv0 == '/')
        {
            bin = argv0 + 1;
        }
        argv0++;
    }
    printf("\
usage:\n\
    %s",
           bin);
    if (cli->nopts > 0)
    {
        printf(" [options]");
    }
    for (i = 0; i < cli->nargs; i++)
    {
        printf(" <%s>", cli->args[i].name);
    }
    print_args(cli);
    print_opts(cli);
    printf("\n");
}

const cli_opt_t *opt_parse(size_t argc, char *const *argv, cli_opt_t *opt, size_t *pidx)
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
                opt = find_opt_l(argv[i] + 2, cli);
                if (!opt_parse(argc, argv, opt, &pidx))
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
                    opt = find_opt_s(argv[i][j++], cli);
                    if (!opt_parse(argc, argv, opt, &pidx))
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