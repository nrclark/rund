#include "config.h"

#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_pvt_libparse.h"
#include "libparse.h"

#ifndef VERSION
#define VERSION "0.0.0"
#endif

/*----------------------------------------------------------------------------*/

static char progname[NAME_MAX + 1] = "(progname)";
static char version[NAME_MAX + 1] = VERSION;

static const char *help = "(help not provided)";
static const char *tagline = "";
static char canonical_name[NAME_MAX + 1] = {0};
static const char *usage = "(usage not provided)";

/*----------------------------------------------------------------------------*/

const char * parser_get_progname(void)
{
    return progname;
}

void parser_set_help(const char *help_string)
{
    help = help_string;
}

void parser_set_usage(const char *usage_string)
{
    usage = usage_string;
}

void parser_set_tagline(const char *tagline_string)
{
    tagline = tagline_string;
}

void parser_set_canonical_name(const char *name_string)
{
    int length = strnlen(name_string, sizeof(canonical_name) - 1);
    strncpy(canonical_name, name_string, (size_t)length);
    canonical_name[length] = '\x00';
}

void exit_usage(void)
{
    fprintf(stderr, "usage: ");
    fprintf(stderr, "%s %s", progname, usage);
    fputs("\n", stderr);
    exit(1);
}

void exit_missing_arg(const char *optname)
{
    fprintf(stderr, "error: missing argument for [%s].\n", optname);

    if (optname[0] != '-') {
        exit_usage();
    }

    fprintf(stderr, "Try %s --help for more information.\n", progname);
    exit(1);
}

void exit_unknown_arg(const char *optname)
{
    fprintf(stderr, "error: unknown argument [%s].\n", optname);
    fprintf(stderr, "Try %s --help for more information.\n", progname);
    exit(1);
}

void exit_help(void)
{
    fprintf(stdout, "usage: %s %s\n", progname, usage);
    fputs(help, stdout);
    fprintf(stdout, "\n");

    if (tagline != NULL) {
        if (tagline[0] != '\x00') {
            fprintf(stdout, "\n%s: %s\n", progname, tagline);
        }
    }

    exit(0);
}

void exit_version(void)
{
    const char *name = progname;

    if (canonical_name[0] != '\x00') {
        name = canonical_name;
    }

    fprintf(stdout, "%s version: %s\n", name, version);

    if (tagline != NULL) {
        if (tagline[0] != '\x00') {
            fprintf(stdout, "%s - %s\n", name, tagline);
        }
    }

    exit(0);
}

void exit_badvalue(const char *shortarg, const char *longarg,
                   const char *value)
{
    char select = ((shortarg != NULL) << 1) + (longarg != NULL);

    switch (select) {
        case 0:
            parser_exit_error(false, ": invalid value [%s].", value);
            break;

        case 1:
            parser_exit_error(false, ": invalid value [%s] for %s.", value,
                              longarg);
            break;

        case 2:
            parser_exit_error(false, ": invalid value [%s] for %s.", value,
                              shortarg);
            break;

        case 3:
            parser_exit_error(false, ": invalid value [%s] for %s/%s.",
                              value, shortarg, longarg);
            break;

        default:
            break;
    }
}

/*----------------------------------------------------------------------------*/

void parser_exit_error(bool append_usage, const char *message, ...)
{
    va_list args;

    fprintf(stderr, "error: %s", progname);

    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);

    fputs("\n", stderr);

    if (append_usage) {
        exit_usage();
    }

    exit(1);
}

void parser_stderr_msg(const char *message, ...)
{
    va_list args;

    fprintf(stderr, "%s", progname);

    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);

    fputs("\n", stderr);
}

void parser_init_progname(char *argv0)
{
    /** @brief Copies the non-directory portion of argv[0] into 'progname'. */

    int length = strnlen(basename(argv0), sizeof(progname) - 1);
    strncpy(progname, basename(argv0), (size_t)length);
    progname[length] = '\x00';
}

void parser_set_version(const char *prog_version)
{
    /** @brief Copies the non-directory portion of argv[0] into 'progname'. */

    int length = strnlen(prog_version, sizeof(version) - 1);
    strncpy(version, prog_version, (size_t)length);
    version[length] = '\x00';
}

/*----------------------------------------------------------------------------*/
