#ifndef _LIBPATH_H_
#define _LIBPATH_H_

#include <sys/stat.h>

int path_mkdirs(const char *path, mode_t mode);
int path_findprog(const char *name, char *output);

#endif
