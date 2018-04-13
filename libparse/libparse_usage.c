#include "config.h"

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "libparse.h"

enum {
    bufsize = 2 * NAME_MAX + 5,
};

static unsigned int maxwidth = 0;
static char buffer[bufsize + 1] = {0};
static unsigned int wrapchunk_count = 0;

static unsigned int get_terminal_width(void)
{
#if defined TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    return ts.ts_cols;
#elif defined TIOCGWINSZ
    struct winsize ts;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
    return ts.ws_col;
#else
    return 80;
#endif
}

static void print_wrapchunk(const char *data, FILE *outfile)
{
    size_t length = strlen(data);

    if ((length + wrapchunk_count) >= maxwidth) {
        fputs("\n", outfile);
        wrapchunk_count = 0;
    }

    fputs(data, outfile);
    wrapchunk_count += length;
}

static void snupper_decr(char **dest, int *space, const char *src)
{
    char *output = *dest;
    int remaining = *space;
    int count = 0;

    while (*src == '-') {
        src++;
    }

    while ((remaining > 0) && (*src != '\x00')) {
        *(output++) = toupper(*(src++));
        remaining--;
        count++;
    }

    *space -= count;
    *dest += count;
}

static void snprintf_decr(char **dest, int *space, const char *fmt, ...)
{
    int result;
    va_list args;

    va_start(args, fmt);
    result = vsnprintf(*dest, (size_t)(*space), fmt, args);
    va_end(args);

    if (result > 0) {
        if (result >= *space) {
            result = *space;
        }
        *space -= result;
        *dest += result;
    }
}

static void usage_optarg_print(struct arg_t *arg, FILE *outfile)
{
    char *ptr = buffer;
    int space = bufsize;
    const char *opt = arg->shortopt;

    if (opt == NULL) {
        opt = arg->longopt;
    }

    if (opt == NULL) {
        return;
    }

    snprintf_decr(&ptr, &space, " [%s", opt);

    if (arg->longopt != NULL) {
        switch (arg->type) {
            case arg_optional:
                snprintf_decr(&ptr, &space, " [");
                snupper_decr(&ptr, &space, arg->longopt);
                snprintf_decr(&ptr, &space, "]");
                break;

            case arg_required:
                snprintf_decr(&ptr, &space, " ");
                snupper_decr(&ptr, &space, arg->longopt);
                break;

            case arg_none:
            default:
                break;
        }
    }

    snprintf_decr(&ptr, &space, "]");
    *ptr = '\x00';
    print_wrapchunk(buffer, outfile);
}

static void usage_posarg_print(struct arg_t *arg, FILE *outfile)
{
    char *ptr = buffer;
    int space = bufsize;

    switch (arg->type) {
        case arg_optional:
            snprintf_decr(&ptr, &space, " [");
            snupper_decr(&ptr, &space, arg->longopt);
            snprintf_decr(&ptr, &space, "]");
            break;

        case arg_required:
            snprintf_decr(&ptr, &space, " ");
            snupper_decr(&ptr, &space, arg->longopt);
            break;

        case arg_none:
        default:
            break;
    }

    *ptr = '\x00';
    print_wrapchunk(buffer, outfile);
}

static void print_usage(struct arg_t args[], unsigned int nargs, FILE *outfile)
{
    wrapchunk_count = 0;
    maxwidth = get_terminal_width();

    print_wrapchunk("usage: ", outfile);
    print_wrapchunk(parser_get_progname(), outfile);

    for (unsigned int x = 0; x < nargs; x++) {
        if (args[x].shortopt) {
            usage_optarg_print(args + x, outfile);
            continue;
        }
        if (args[x].longopt) {
            if (args[x].longopt[0] == '-') {
                usage_optarg_print(args + x, outfile);
            }
        }
    }

    for (unsigned int x = 0; x < nargs; x++) {
        if (args[x].shortopt) {
            continue;
        }
        if (args[x].longopt) {
            if (args[x].longopt[0] != '-') {
                usage_posarg_print(args + x, outfile);
            }
        }
    }
}

void parser_stderr_usage(struct arg_t args[], unsigned int nargs)
{
    print_usage(args, nargs, stderr);
}

void parser_stdout_usage(struct arg_t args[], unsigned int nargs)
{
    print_usage(args, nargs, stdout);
}
