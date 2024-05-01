/**
 * @file cli.h
 * @author Rob Griffith
 */

#ifndef CLI_H
#define CLI_H

#include "bufferedio.h"

#include <stddef.h>

/**
 * @struct cli_arg
 * @brief command line input required argument context (definition and parsed value)
 * @typedef cli_arg_t
 */
typedef struct cli_arg
{
    const char *name; /** name of the argument */
    const char *desc; /** description of the argument */
    const char *val;  /** parsed value of the argument, NULL if not found */
} cli_arg_t;

/**
 * @struct cli_opt
 * @brief command line input option context (definition and parsed value)
 * @typedef cli_opt_t
 */
typedef struct cli_opt
{
    const char name_s;  /** short -name of the option, '\0' for no short option */
    const char *name_l; /** long --name of the option */
    const char *desc;   /** description of the option */
    const char *param;  /** parameter name required with the option, NULL for no parameter */
    const char *def;    /** default value of the option, NULL for none/false */
    const char *val;    /** parsed value of the option parameter, NULL if not found */
} cli_opt_t;

/**
 * @struct cli
 * @brief command line input context (definition and parsed values)
 * @typedef cli_t
 */
typedef struct cli
{
    const char *cmd; /** the command name, parsed if not manually set */
    size_t nargs;    /** the number of arguments, sizeof(args) / sizeof(cli_arg_t) */
    cli_arg_t *args; /** the array of arguments to parse */
    size_t nopts;    /** the number of options, sizeof(opts) / sizeof(cli_opt_t) */
    cli_opt_t *opts; /** the array of options to parse */
} cli_t;

/**
 * @brief print command line usage information to stderr
 *
 * @param cli command line input context
 */
void cli_print_usage(const cli_t *cli);

/**
 * @brief get command line input required argument context by name
 *
 * Provided as alternative to accessing through @ref cli::args with known index.
 *
 * @param cli command line input context
 * @param name name of the argument
 * @return command line input requried argument context (NULL if not found)
 */
cli_arg_t *cli_get_arg(cli_t *cli, const char *name);

/**
 * @brief get command line input option context by long name
 *
 * Provided as alternative to accessing through @ref cli::opts with known index.
 * Long names are used as option contexts not requiring short names.
 *
 * @param cli command line input context
 * @param name_l long --name of the option
 * @return command line input option context (NULL if not found)
 */
cli_opt_t *cli_get_opt(cli_t *cli, const char *name_l);

/**
 * @brief parse command line given command line input context
 *
 * Error message is printed to stderr and NULL is returned upon any error.
 * Use @ref cli_print_usage for help or upon error.
 *
 * @param argc number of command line arguments (values)
 * @param argv command line argument (value) strings
 * @param[inout] cli command line input context to use and populate
 * @return the provided command line context, NULL if error
 */
const cli_t *cli_parse(size_t argc, char *const *argv, cli_t *cli);

/**
 * @brief parse command line from buffered I/O context given CLI context
 *
 * Reads and tokenizes strings from buffered I/O context until newline or EOF.
 * If @def cli::cmd is not set, value is parsed from first token.
 * Generally, the first token should be parsed separately and used to select the relevant CLI context.
 * Tokenized strings are saved into separate provided buffer.
 * @def cli_parse used to parse tokens.
 * If error, buffered I/O context status should be checked to determine if I/O error or invalid input.
 *
 * @param[inout] bio buffered I/O context
 * @param[inout] buf buffer to store tokens parsed from command line input line
 * @param[inout] cli command line input context to use and populate
 * @return > 0 if sucessful, 0 if EOF, < 0 if error
 */
int cli_parse_line(bufferedio_t *bio, buffer_t *buf, cli_t *cli);

#endif