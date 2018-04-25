#include "config.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "libconfig.h"
#include "libnointr.h"
#include "libpath.h"
#include "libparse.h"

#include "rund_paths.h"

static const char rcfile[] = ".rundrc";
static const char conffile[] = "rund.conf";
static const char sysconfdir[] = "etc";
static const char runstatedir[] = "/var/run";

static int get_user(char *output, size_t maxlen)
{
    int result;
    struct passwd *pw;
    uid_t uid;

    uid = getuid();
    pw = getpwuid_nointr(uid);

    if (pw == NULL) {
        result = snprintf(output, maxlen, "%ju", (uintmax_t)(uid));
        if (result < 0) {
            return result;
        }
        return (((unsigned int) result + 1) >= maxlen);
    }

    return path_strncpy(output, pw->pw_name, maxlen);
}

static int default_statedir(char *output, size_t maxlen)
{
    char folder[PATH_MAX + 1];
    char name[LOGIN_NAME_MAX + 1];
    int result = get_user(name, sizeof(name));

    if (result != 0) {
        return result;
    }

    result = path_join(folder, runstatedir, parser_get_progname(),
                       sizeof(folder));
    if (result != 0) {
        return result;
    }

    result = path_join(output, folder, name, maxlen);
    return result;
}

static int config_statedir(const char *folder, const char *configfile,
                           char *output, size_t maxlen)
{
    char *value;
    char filename[PATH_MAX + 1];

    int result = path_join(filename, folder, configfile, sizeof(filename));

    if (result != 0) {
        return result;
    }

    if (path_readable(filename) == 0) {
        result = config_lookup(filename, NULL, "statedir", &value);

        if (result == 0) {
            result = path_strncpy(output, value, maxlen);
            free(value);
            return result;
        }
    }

    return -1;
}

int rund_statedir_get(char *output, size_t maxlen, bool system_only)
{
    const char *homedir;
    char *value;

    if (system_only == false) {
        value = getenv("STATEDIR");
        if (value != NULL) {
            return path_strncpy(output, value, maxlen);
        }

        homedir = path_homedir();

        if (config_statedir(homedir, rcfile, output, maxlen) == 0) {
            return 0;
        }
    }

    if (config_statedir(sysconfdir, conffile, output, maxlen) == 0) {
        return 0;
    }

    return default_statedir(output, maxlen);
}
