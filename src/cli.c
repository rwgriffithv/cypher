/**
 * @file cli.c
 * @author Rob Griffith
 */

#include "cli.h"

#include <stdio.h>
#include <string.h>

const char *get_opt_type_str(opttype_t type)
{
    switch (type)
    {
    case FLAG:
        return "FLAG";
    case PARAM:
        return "PARAMETER";
    }
}

option_t *find_opt_l(const char *name_l, cli_t *cli)
{
    for (int i = 0; i < cli->nopts; i++)
    {
        if (strcmp(name_l, cli->opts[i].name_l) == 0)
        {
            return cli->opts + i;
        }
    }
    fprintf(stderr, "invalid unknown option --%s\n", name_l);
    return NULL;
}

option_t *find_opt_s(const char name_s, cli_t *cli)
{
    for (int i = 0; i < cli->nopts; i++)
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
    if (cli->nargs <= 0)
    {
        return;
    }
    printf("\n\
arguments:");
    for (int i = 0; i < cli->nargs; i++)
    {
        printf("\n\
    %s\n\
        %s",
               cli->args[i].name, cli->args[i].desc);
    }
}

void print_opts(const cli_t *cli)
{
    if (cli->nopts <= 0)
    {
        return;
    }
    printf("\n\
options:");
    for (int i = 0; i < cli->nopts; i++)
    {
        const option_t *opt = cli->opts + i;
        printf("\n\
    --%s",
               opt->name_l);
        if (opt->name_s != '\0')
        {
            printf("\n\
    -%c",
                   opt->name_s);
        }
        printf("\n\
        %s\n\
        TYPE: %s",
               opt->desc, get_opt_type_str(opt->type));
        if (opt->val)
        {
            printf("\n\
        DEFAULT: %s",
                   opt->val);
        }
    }
}

void print_usage(const char *bin, const cli_t *cli)
{
    printf("\
usage:\n\
    %s",
           bin);
    if (cli->nopts > 0)
    {
        printf(" [options]");
    }
    for (int i = 0; i < cli->nargs; i++)
    {
        printf(" <%s>", cli->args[i].name);
    }
    print_args(cli);
    print_opts(cli);
    printf("\n");
}

int opt_parse(int argc, char **argv, option_t *opt, int *pidx)
{
    if (!opt)
    {
        return -1;
    }
    switch (opt->type)
    {
    case FLAG:
        opt->val = "true";
        break;
    case PARAM:
        if (*pidx >= argc)
        {
            /* missing required value for option */
            fprintf(stderr, "missing required argument for parameter-option --%s (-%c)\n", opt->name_l, opt->name_s);
            return -1;
        }
        opt->val = argv[*pidx];
        *pidx = *pidx + 1;
        break;
    }
    return 0;
}

int cli_parse(int argc, char **argv, cli_t *cli)
{
    int i_arg = 0;
    int nopts = 0;
    int i = 1; /* skip binary name */
    while (i < argc)
    {
        if (argv[i][0] == '-' && argv[i][1] != '\0')
        {
            /* parse option */
            option_t *opt = NULL;
            int pidx = i + 1;
            if (argv[i][1] == '-')
            {
                /* parse full option */
                opt = find_opt_l(argv[i] + 2, cli);
                if (opt_parse(argc, argv, opt, &pidx) < 0)
                {
                    return -1;
                }
                nopts++;
            }
            else
            {
                /* parse short option(s) */
                int j = 1;
                while (argv[i][j] != '\0')
                {
                    opt = find_opt_s(argv[i][j++], cli);
                    if (opt_parse(argc, argv, opt, &pidx) < 0)
                    {
                        return -1;
                    }
                    nopts++;
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
            fprintf(stderr, "invalid extra argument \"%s\" (exactly %d arguments required)\n", argv[i], cli->nargs);
            return -1;
        }
    }
    if (i_arg < cli->nargs)
    {
        /* missing required arguments */
        fprintf(stderr, "only parsed %d of required %d arguments\n", i_arg, cli->nargs);
        return -1;
    }
    return nopts;
}