/**
 * @file cli.h
 * @author Rob Griffith
 */

#ifndef CLI_H
#define CLI_H

#include <string.h>

typedef struct cli_opt
{
    const char name_s;
    const char *name_l;
    const char *desc;
    const char *param;
    const char *def;
    const char *val;
} cli_opt_t;

typedef struct cli_arg
{
    const char *name;
    const char *desc;
    const char *val;
} cli_arg_t;

typedef struct cli
{
    size_t nopts;
    cli_opt_t *opts;
    size_t nargs;
    cli_arg_t *args;
} cli_t;

void print_usage(const char *argv0, const cli_t *cli);

const cli_t *cli_parse(size_t argc, char *const *argv, cli_t *cli);

#endif