#include "config.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "libparse/libparse.h"
#include "librund/libconfig.h"
#include "librund/libnointr.h"
#include "librund/libpath.h"

static const char rcfile[] = ".rundrc";
static const char conffile[] = "rund.conf";
static const char sysconfdir[] = "etc";
static const char runstatedir[] = "/var/run";

static int get_euser(char *output, size_t maxlen)
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
    int result = get_euser(name, sizeof(name));

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

int get_statedir(char *output, size_t maxlen, bool system_only)
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
    int result = get_statedir(statedir, sizeof(statedir) - 1, system_only);
    printf("result: [%d], statedir: [%s]\n", result, statedir);
    return 0;
}
