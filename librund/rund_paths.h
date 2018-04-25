#ifndef _RUND_PATHS_H_
#define _RUND_PATHS_H_

#include "config.h"

#include <stdbool.h>
#include <sys/types.h>

int rund_statedir_get(char *output, size_t maxlen, bool system_only);

#endif
