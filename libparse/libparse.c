#include "config.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "_pvt_libparse.h"
#include "libparse.h"

static const char help_longarg[] = "--help";
static const char version_longarg[] = "--version";

/*-------------------------- Short Helper Functions --------------------------*/

static inline void safe_increment(unsigned int *count)
{
    /* Increments the value stored in *count unless count is NULL. */

    if (count != NULL) {
        *(count) += 1;
    }
}

static bool is_positional(const char *arg)
{
    /* Returns true if the argument (from argv) is positional/non-option. */

    if (arg == NULL) {
        return false;
    }

    return (arg[0] != '-');
}

static void move_to_end(char *argv[], unsigned int index)
{
    /* Moves an arg in argv to the end of the array. */

    char *temp = argv[index];
    unsigned int x;

    for (x = index; argv[x + 1] != NULL; x++) {
        argv[x] = argv[x + 1];
    }

    argv[x] = temp;
}

static int find_argument(char *argv[], const char *target)
{
    /* Finds the index of an arg in argv. */

    for (int x = 0; argv[x] != NULL; x++) {
        if (argv[x] == target) {
            return x;
        }
    }

    return -1;
}

static bool is_positional_argt(struct arg_t *arg)
{
    /* Returns true if an arg_t argument description is configured for
     * a positional argument. */

    if (arg->longopt == NULL) {
        return false;
    }

    return (arg->shortopt == NULL) && (arg->longopt[0] != '-');
}

/*--------------------------- Long Helper Functions --------------------------*/

static int longopt_index(struct arg_t args[], unsigned int nargs, char *arg)
{
    /* Returns -1 if *arg is NULL or if it isn't a long-option. Otherwise,
     * scans for the option in args[] and returns the index into args[].
     * If *arg is an unknown long-option, exit_unknown_arg() is called. */

    if (arg == NULL) {
        return -1;
    }

    if ((arg[0] != '-') || (arg[1] != '-')) {
        return -1;
    }

    for (unsigned int x = 0; x < nargs; x++) {
        int length;

        if (args[x].longopt == NULL) {
            continue;
        }

        if (args[x].longopt[0] != '-') {
            continue;
        }

        length = strnlen(args[x].longopt, argname_maxlen);

        if (strncmp(arg, args[x].longopt, (size_t) length) == 0) {
            if ((arg[length] == '\x00') || (arg[length] == '=')) {
                return (int)x;
            }
        }
    }

    exit_unknown_arg(arg);
    return -1;
}

static int shortopt_index(struct arg_t args[], unsigned int nargs, char arg)
{
    /* Searches through args[] for an entry with a short-option that
     * matches the 'arg' character. Returns the index if a match is found,
     * otherwise exit_unknown_arg() is called. */

    char arg_string[] = "-?";

    for (unsigned int x = 0; x < nargs; x++) {
        if (args[x].shortopt == NULL) {
            continue;
        }

        if (args[x].shortopt[1] == arg) {
            return (int)x;
        }
    }

    arg_string[1] = arg;
    exit_unknown_arg(arg_string);
    return -1;
}

static void load_value(struct arg_t *arg, char *optarg)
{
    /** Loads optarg into *arg.value, assuming that arg.value isn't NULL.
     * Will attempt to use a transformer if one is present, otherwise it'll
     * treat arg.value as a char** and load optarg into its contents. */

    int result;

    if (arg->value == NULL) {
        return;
    }

    if (arg->transformer == NULL) {
        *((char **)(arg->value)) = optarg;
        return;
    }

    result = arg->transformer(optarg, arg->value);

    if (result != 0) {
        exit_badvalue(arg->shortopt, arg->longopt, optarg);
    }
}

static int process_long(struct arg_t *arg, char **argv)
{
    /* Processes a chunk of argv against a pre-selected argument. Increments
     * the count variable if needed, and tries to load arg->value if
     * appropriate. Will try to consume the next entry in argv if required.
     * Returns the number of additional arguments consumed by this function
     * (0 or 1). */

    safe_increment(arg->count);

    if (arg->type == arg_none) {
        return 0;
    }

    for (unsigned int x = 0; argv[0][x] != '\x00'; x++) {
        if (argv[0][x] == '=') {
            load_value(arg, argv[0] + x + 1);
            return 0;
        }
    }

    if (is_positional(argv[1])) {
        load_value(arg, argv[1]);
        return 1;
    }

    if (arg->type == arg_required) {
        exit_missing_arg(arg->longopt);
    }

    return 0;
}

static int process_short(struct arg_t args[], unsigned int nargs, char **argv)
{
    /* Processes an argv entry that holds one or more short options. Increments
     * each detected option's count variable. The first value-consuming option
     * will eat the rest of the argument (or the next argument in argv if it's
     * the last char of the current one). Returns the number of additional
     * arguments consumed by this function (0 or 1). */

    for (unsigned int x = 1; argv[0][x] != '\x00'; x++) {
        if (argv[0][x] == 'h') {
            exit_help();
        }

        if (argv[0][x] == 'V') {
            exit_version();
        }

        int index = shortopt_index(args, nargs, argv[0][x]);
        safe_increment(args[index].count);

        if (args[index].type == arg_none) {
            continue;
        }

        if (argv[0][x + 1] != '\x00') {
            if (argv[0][x + 1] == '=') {
                x++;
            }

            load_value(args + index, argv[0] + x + 1);
            return 0;
        }

        if (is_positional(argv[1])) {
            load_value(args + index, argv[1]);
            return 1;
        }

        if (args[index].type == arg_required) {
            char arg_str[] = "-?";
            arg_str[1] = argv[0][x];
            exit_missing_arg(arg_str);
        }
    }

    return 0;
}

static void verify_required_posargs(struct arg_t args[], unsigned int nargs)
{
    /* Scans through the args[] array and verifies that all positional
     * arguments of type arg_required were filled successfully. Calls
     * exit_missing_arg() if a missing argument is detected. */

    for (unsigned int x = 0; x < nargs; x++) {
        if (is_positional_argt(args + x) == false) {
            continue;
        }

        if (args[x].type == arg_required) {
            if (*(char **)args[x].value == NULL) {
                exit_missing_arg(args[x].longopt);
            }
        }
    }
}

/*--------------------------- Main Parsing Function --------------------------*/

int parser_run(struct arg_t args[], unsigned int nargs, int argc, char *argv[],
               bool stop)
{
    /* Scans through argv and parses it according to the options list provided
     * in args[]. Long, short, and positional options are all detected.
     *
     * If the '--' argument is found, all further arguments are treated as
     * remainder arguments.
     *
     * If 'stop' is true, parser_run() stops processing as soon as it sees a
     * non-option value and returns argv index of the value.
     *
     * Otherwise it will scan the rest of argv for any additional options,
     * move all positional arguments to the end of argv, and return the
     * argv index of the first one of these. */

    char *extra_args[argc];
    unsigned int shadow_index = 0;
    unsigned int posindex = 0;
    unsigned int count;

    parser_init_progname(argv[0]);

    for (unsigned int x = 0; x < nargs; x++) {
        /* Resets all 'count' variables, and also 'value' fields on
         * positional arguments. */

        if (args[x].count != NULL) {
            *args[x].count = 0;
        }

        if (is_positional_argt(args + x) == false) {
            continue;
        }

        if (args[x].value != NULL) {
            *((char **)args[x].value) = NULL;
        }
    }

    for (count = 1; count < (unsigned int)argc; count++) {
        /* Main processing loop. */

        if (argv[count] == NULL) {
            break;
        }

        if (strcmp(argv[count], "--") == 0) {
            for (unsigned int x = count + 1; x < (unsigned int)argc; x++) {
                extra_args[shadow_index++] = argv[x];
            }

            break;
        }

        if (strcmp(argv[count], help_longarg) == 0) {
            exit_help();
        }

        if (strcmp(argv[count], version_longarg) == 0) {
            exit_version();
        }

        int index = longopt_index(args, nargs, argv[count]);

        if (index != -1) {
            int extra = process_long(args + index, argv + count);
            count += (unsigned int)extra;
            continue;
        }

        if (argv[count][0] == '-') {
            int extra = process_short(args, nargs, argv + count);
            count += (unsigned int)extra;
            continue;
        }

        bool stored = false;

        for (unsigned int x = posindex; x < nargs; x++) {
            if (args[x].shortopt || (strncmp(args[x].longopt, "--", 2) == 0)) {
                continue;
            }

            safe_increment(args[x].count);
            load_value(args + x, argv[count]);

            posindex = x + 1;
            stored = true;
            break;
        }

        if (stored == false) {
            if (stop) {
                verify_required_posargs(args, nargs);
                return (int)count;
            }

            extra_args[shadow_index++] = argv[count];
        }
    }

    for (unsigned int x = 0; x < shadow_index; x++) {
        int index = find_argument(argv, extra_args[x]);

        if (index < 0) {
            break;
        }

        move_to_end(argv, (unsigned int)index);
    }

    verify_required_posargs(args, nargs);
    return argc - (int)shadow_index;
}
