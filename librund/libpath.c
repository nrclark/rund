#include "config.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libpath.h"

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

int path_mkdirs(const char *path, mode_t mode)
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

int path_findprog(const char *name, char *output)
{
    const char *path = getenv("PATH");

    unsigned int name_length = strnlen(name, NAME_MAX);
    unsigned int path_length;
    unsigned int start = 0;
    unsigned int end = 0;

    for (unsigned int x = 0; name[x] != '\x00'; x++) {
        if (name[x] == '/') {
            strncpy(output, name, PATH_MAX);
            output[PATH_MAX] = '\x00';
            return 0;
        }
    }

    if (path != NULL) {
        if (path[0] == '\x00') {
            path = NULL;
        }
    }

    if (path == NULL) {
        path = "/usr/bin:/bin";
    }

    while (1) {
        if ((path[end] == ':') || (path[end] == '\x00')) {
            path_length = end - start;

            if ((path_length + name_length + 1) > PATH_MAX) {
                path_length = PATH_MAX - (name_length + 1);
            }

            memcpy(output, path + start, path_length);
            output[path_length] = '/';
            memcpy(output + path_length + 1, name, name_length);
            output[name_length + path_length + 1] = '\x00';
            start = end + 1;

            if (access(output, R_OK | X_OK) == 0) {
                return 0;
            }
        }

        if (path[end] == '\x00') {
            break;
        }
        end++;
    }

    memcpy(output, "./", 2);
    memcpy(output + 2, name, name_length);
    output[2 + name_length] = '\x00';

    if (access(output, R_OK | X_OK) == 0) {
        return 0;
    }

    return -1;
}
