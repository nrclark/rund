#ifndef _LIBCONFIG_H_
#define _LIBCONFIG_H_

int config_lookup(const char *filename, const char *section, const char *key,
                  char **value);

#endif
