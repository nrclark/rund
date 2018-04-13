#include <stdint.h>
#include <stdio.h>

#include "libparse.h"
#include "libparse_transforms.h"

/*----------------------------------------------------------------------------*/

static struct opts {
    const char *input;
    unsigned int quiet;
    const char *conf;
    unsigned int config_count;
    const char *argname;
    unsigned int verbose;
    uint64_t offset;
    uint64_t length;
    const char *fuck;
    unsigned int shit;
    const char *filter;
    unsigned int filter_count;
} opts = {
    .input = "",
    .conf = "default",
    .argname = "",
    .fuck = "",
    .filter = ""
};

static void print_opts(void)
{
    printf("Input file: [%s]\n", opts.input);
    printf("Quietness: [%u]\n", opts.quiet);
    printf("Config file: [%s] (count: %u)\n", opts.conf, opts.config_count);
    printf("Argument name: [%s]\n", opts.argname);
    printf("Verboseness: [%u]\n", opts.verbose);
    printf("File offset: [%u]\n", (unsigned int)opts.offset);
    printf("File length: [%u]\n", (unsigned int)opts.length);
    printf("Shits: [%u]\n", opts.shit);
    printf("Fucks name: [%s]\n", opts.fuck);
    printf("Filter: [%s] (count: %u)\n", opts.filter, opts.filter_count);
}

int main(int argc, char *argv[])
{
    unsigned int nargs;
    int remaining;

    struct arg_t options[] = {
        {"-i", "--input", &opts.input, NULL, NULL, arg_required},
        {"-q", NULL, NULL, &opts.quiet, NULL, arg_none},
        {"-c", "--config", &opts.conf, &opts.config_count, NULL, arg_optional},
        {"-v", "--verbose", NULL, &opts.verbose, NULL, arg_none},
        {"-o", "--offset", &opts.offset, NULL, arg_uint64, arg_required},
        {NULL, "--fuck", &opts.fuck, NULL, NULL, arg_required},
        {NULL, "--shit", NULL, &opts.shit, NULL, arg_none},
        {NULL, "ARGNAME", &opts.argname, NULL, NULL, arg_required},
        {NULL, "LENGTH", &opts.length, NULL, arg_uint64, arg_required},
        {NULL, "FILTER", &opts.filter, &opts.filter_count, NULL, arg_optional},
    };

    nargs = sizeof(options) / sizeof(options[0]);

    parser_set_version("1.23.314");
    parser_stdout_usage(options, nargs);
    printf("\n------------\n");
    remaining = parser_run(options, nargs, argc, argv, false);
    print_opts();

    printf("Remaining arguments:\n");

    for (int x = remaining; x < argc; x++) {
        printf("%s\n", argv[x]);
    }

    return 0;
}
