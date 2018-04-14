#include "config.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libnointr.h"
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

static int check_exec(const char *restrict dir, size_t dirlen,
                      const char *restrict file, size_t filelen,
                      char *restrict buffer, size_t maxlen)
{
    unsigned char termdir;
    int result;

    if (maxlen == 0) {
        return -1;
    }

    if (dirlen == 0) {
        termdir = 0;
    } else {
        termdir = (dir[dirlen - 1] != '/');
    }

    if ((dirlen + termdir + filelen + 1) > maxlen) {
        return -1;
    }

    memcpy(buffer, dir, dirlen);
    buffer += dirlen;

    if (termdir) {
        *(buffer++) = '/';
    }

    memcpy(buffer, file, filelen);
    buffer[filelen] = '\x00';
    result = access(buffer, R_OK | X_OK);

    if (result != 0) {
        buffer[0] = '\x00';
    }

    return result;
}

int path_findprog(const char *restrict name, char *restrict dest,
                  size_t maxlen)
{
    const char *path;
    const char *dir;
    unsigned int dirlen;
    unsigned int namelen = strnlen(name, PATH_MAX);

    for (unsigned int x = 0; name[x] != '\x00'; x++) {
        if (name[x] == '/') {
            return check_exec(NULL, 0, name, namelen, dest, maxlen);
        }
    }

    if (namelen > NAME_MAX) {
        return -1;
    }

    path = getenv("PATH");

    if (path != NULL) {
        if (path[0] == '\x00') {
            path = NULL;
        }
    }

    if (path == NULL) {
        path = "/usr/bin:/bin";
    }

    dir = path;
    path += 1;

    while (1) {
        if ((*path == ':') || (*path == '\x00')) {
            dirlen = path - dir;
            if (check_exec(dir, dirlen, name, namelen, dest, maxlen) == 0) {
                return 0;
            }
            dir = path + 1;
        }

        if ((*path++) == '\x00') {
            break;
        }
    }

    return check_exec(".", 1, name, namelen, dest, maxlen);
}

int path_readable(const char *name)
{
    if (name == NULL) {
        return -1;
    }

    if (access(name, R_OK) == 0) {
        return 0;
    }

    return -1;
}

const char * path_homedir(void)
{
    const char *dir = getenv("HOME");
    struct passwd *result;

    if (dir != NULL) {
        return dir;
    }

    result = getpwuid_nointr(getuid());

    if (result == NULL) {
        return NULL;
    }

    return result->pw_dir;
}

int path_join(char *restrict dest, const char *restrict dir,
              const char *restrict file, size_t maxlen)
{
    size_t dir_len;
    size_t file_len;
    unsigned char need_slash;

    dir_len = strnlen(dir, maxlen);
    file_len = strnlen(file, maxlen);
    need_slash = (dir[dir_len - 1] != '/');

    if ((dir_len + file_len + need_slash + 1) > maxlen) {
        if (maxlen != 0) {
            dest[0] = '\x00';
        }
        return -1;
    }

    memcpy(dest, dir, dir_len);
    dest += dir_len;

    if (need_slash) {
        *(dest++) = '/';
    }

    memcpy(dest, file, file_len);
    dest[file_len] = '\x00';

    return 0;
}

int path_strncpy(char *restrict dest, const char *restrict src, size_t maxlen)
{
    size_t len = strnlen(src, maxlen);

    if (len >= maxlen) {
        if (maxlen != 0) {
            dest[0] = '\x00';
        }
        return -1;
    }

    memcpy(dest, src, len);
    dest[len] = '\x00';
    return 0;
}
