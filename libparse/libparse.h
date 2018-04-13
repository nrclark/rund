#ifndef _LIBPARSE_H_
#define _LIBPARSE_H_

#include <stdbool.h>

enum {argname_maxlen = 128};

typedef int (*parser_transformer_t)(const char *input, void *output);

typedef enum argtype_t {
    arg_none = 0,
    arg_required = 1,
    arg_optional = 2
} argtype_t;

struct arg_t {
    const char *shortopt;
    const char *longopt;
    void *value;
    unsigned int *count;
    parser_transformer_t transformer;
    argtype_t type;
};

/*----------------------------------------------------------------------------*/

void parser_exit_error(bool append_usage, const char *message, ...);
void parser_stderr_msg(const char *message, ...);

void parser_set_help(const char *help_string);
void parser_set_usage(const char *usage_string);
void parser_set_version(const char *prog_version);

void parser_set_tagline(const char *tagline_string);
void parser_set_canonical_name(const char *name_string);

const char * parser_get_progname(void);

/*----------------------------------------------------------------------------*/

/* Returns the index of the first unparsed argument in argv (potentially
 * after shuffling argv). If 'stop' is true, the first non-option argument
 * immediately stops processing and is the first entry in the result. */

int parser_run(struct arg_t args[], unsigned int nargs, int argc, char *argv[],
               bool stop);

#endif
