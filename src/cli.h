/**
 * @file cli.h
 * @author Rob Griffith
 */

#ifndef CLI_H
#define CLI_H

#include <string.h>

typedef enum
{
    FLAG,
    PARAM
} opttype_t;

typedef struct option
{
    const char name_s;
    const char *name_l;
    const char *desc;
    const opttype_t type;
    const char *val;
} option_t;

typedef struct argument
{
    const char *name;
    const char *desc;
    const char *val;
} argument_t;

typedef struct cli
{
    const int nopts;
    option_t *opts;
    const int nargs;
    argument_t *args;
} cli_t;

void print_usage(const char* bin, const cli_t *cli);

int cli_parse(int argc, char **argv, cli_t *cli);

#endif