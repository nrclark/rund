#ifndef _LIBPATH_H_
#define _LIBPATH_H_

#include "config.h"
#include <sys/types.h>

int path_mkdirs(const char *path, mode_t mode);

int path_findprog(const char *restrict name, char *restrict dest,
                  size_t maxlen);

int path_readable(const char *name);

const char * path_homedir(void);

/* Joins a file to a directory stores the result in 'dest', up to 'maxlen'
 * bytes. Returns 0 if everthing copies OK.
 * 
 * If 'dest' is too small to hold the result, 'dest' is set to an empty string
 * and -1 is returned. */

int path_join(char *restrict dest, const char *restrict dir,
              const char *restrict file, size_t maxlen);

/* Copies *src into *dest, up to a total of maxlen characters (including the
 * NUL terminator). Similar to libc's strncpy, except that no pad-bytes are
 * added to the end of *dest.
 * 
 * If the full string copied OK, 0 is returned. Otherwise, *dir is set to
 * an empty string and -1 is returned. */

int path_strncpy(char *restrict dest, const char *restrict src,
                 size_t maxlen);

#endif
