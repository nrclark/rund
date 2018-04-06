#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char mkdirs_errstring[] = "%s: cannot create directory ‘%s’: ";
static const char *progname;

static int mkdirs_lowlevel(char *path, mode_t mode)
{
    unsigned int x;
    int result = -1;

    for (x = 1; path[x] != '\x00'; x++) {
        if (path[x] != '/') {
            continue;
        }

        if ((path[x + 1] != '\x00') && (path[x + 1] != '/')) {
            path[x] = '\x00';

            result = mkdir(path, mode);
            if ((result != 0) && (errno != EEXIST)) {
                fprintf(stderr, mkdirs_errstring, progname, path);
                perror(NULL);
                return result;
            }

            path[x] = '/';
        }
    }

    result = mkdir(path, mode);

    if ((result != 0) && (errno != EEXIST)) {
        fprintf(stderr, mkdirs_errstring, progname, path);
        perror(NULL);
        return result;
    }

    return 0;
}

static int mkdirs(const char *path, mode_t mode)
{
    char buffer[PATH_MAX + 2];
    size_t length = strnlen(path, PATH_MAX + 1);

    if (length > PATH_MAX) {
        fprintf(stderr, "error: pathname to mkdirs() too long\n");
        return -1;
    }

    memcpy(buffer, path, length + 1);
    return mkdirs_lowlevel(buffer, mode);
}

int main(int argc, char **argv)
{
    uintmax_t mode;

    if (argc != 3) {
        fprintf(stderr, "incorrect number of arguments to mkdirs\n");
        exit(1);
    }
    progname = basename(argv[0]);
    sscanf(argv[2], "%jo", &mode);
    return mkdirs(argv[1], (mode_t)(mode));
}
