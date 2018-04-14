#include "config.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#include "libparse/libparse.h"
#include "librund/libconfig.h"
#include "librund/libpath.h"
#include "librund/libnointr.h"

static const char rcfile[] = ".rundrc";
static const char conffile[] = "rund.conf";
static const char sysconfdir[] = "/etc";
static const char runstatedir[] = "/var/run";

static int get_euser(char *output, size_t maxlen)
{
    int result;
    struct passwd *pw;
    uid_t uid;

    uid = geteuid();
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

int get_statedir(char *output, size_t maxlen, bool system_only)
{
    char filename[PATH_MAX + 1];
    char buffer[PATH_MAX + 1];
    const char *homedir;
    char *value;
    int result;

    if (system_only == false) {
        value = getenv("STATEDIR");
        if (value != NULL) {
            return path_strncpy(output, value, maxlen);
        }
    }

    if (system_only == false) {
        homedir = path_homedir();
        result = path_join(filename, homedir, rcfile, sizeof(filename));

        if (path_readable(filename) == 0) {
            result = config_lookup(filename, NULL, "statedir", &value);

            if (result == 0) {
                result = path_strncpy(output, value, maxlen);
                free(value);
                return result;
            }
        }
    }

    result = path_join(filename, sysconfdir, conffile, sizeof(filename));

    if (path_readable(filename) == 0) {
        result = config_lookup(filename, NULL, "statedir", &value);

        if (result == 0) {
            result = path_strncpy(output, value, maxlen);
            free(value);
            return result;
        }
    }

    result = get_euser(buffer, sizeof(buffer));
    if (result != 0) {
        return result;
    }

    result = path_join(filename, runstatedir, parser_get_progname(),
                       sizeof(filename));
    if (result != 0) {
        return result;
    }

    result = path_join(output, filename, buffer, maxlen);
    return result;
}

int main(void)
{
    return 0;
}
