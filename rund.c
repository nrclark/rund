#include "config.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "libparse.h"
#include "libconfig.h"
#include "libnointr.h"
#include "libpath.h"

#include "rund_paths.h"

int main(int argc, char *argv[])
{
    bool system_only = false;
    char statedir[PATH_MAX + 1];

    if (argc > 1) {
        if (argv[1][0] == '1') {
            system_only = true;
        }
    }

    parser_init_progname(argv[0]);
    int result = rund_statedir_get(statedir, sizeof(statedir) - 1, system_only);
    printf("result: [%d], statedir: [%s]\n", result, statedir);
    return 0;
}
