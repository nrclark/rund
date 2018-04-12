#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "_pvt_libparse.h"
#include "libparse.h"
#include "libparse_transforms.h"

int arg_int64(const char *input, void *output)
{
    /** Converts an input argument into an int64 using strtoimax, and stores
     * the result in 'output'. Exits with 0 on success, and -1 on a failure.
     *
     * The following multiplier suffixes are understood and applied to the
     * result: k=2**10, M=2**20, G=2**30, and T=2**40. */

    char *endptr;
    intmax_t value_max = 0;
    int64_t value;
    char mult = 0;

    if (input == NULL) {
        return -1;
    }

    errno = 0;
    value_max = strtoimax(input, &endptr, 0);

    if ((errno != 0) || (endptr == input)) {
        return -1;
    }

    value = value_max;

    if (endptr != NULL) {
        mult = endptr[0];

        if (mult >= 'a') {
            mult -= ('a' - 'A');
        }
    }

    switch (mult) {
        case 0:
            break;

        case 'K':
            value *= 1024;
            break;

        case 'M':
            value *= 1024 * 1024;
            break;

        case 'G':
            value *= 1024 * 1024;
            value *= 1024;
            break;

        case 'T':
            value *= 1024 * 1024;
            value *= 1024 * 1024;
            break;

        default:
            return -1;
    }

    *((int64_t *)(output)) = value;
    return 0;
}

int arg_uint64(const char *input, void *output)
{
    /** Converts an input argument into a uint64 using arg_int64.
     * Strictly speaking this function only works up to 63 bits, but that
     * is plenty for everybody. */

    int64_t buffer;

    int result = arg_int64(input, &buffer);

    if ((result != 0) || (buffer < 0)) {
        return -1;
    }

    *((uint64_t *)output) = (uint64_t)buffer;
    return 0;
}

int arg_int(const char *input, void *output)
{
    char *end = NULL;
    long int result = strtol(input, &end, 10);

    if (end == NULL) {
        return -1;
    }

    if (end == input) {
        return -1;
    }

    if ((result < INT_MIN) || (result > INT_MAX)) {
        return -1;
    }

    *((int *)(output)) = (int)result;
    return 0;
}

int arg_uid(const char *input, void *output)
{
    /** Converts an input argument into a uid_t using arg_int64. Strictly
     * speaking this function only works up to 63 bits, but that is plenty
     * for everybody. */

    int64_t buffer;

    int result = arg_int64(input, &buffer);

    if ((result != 0) || (buffer < 0)) {
        return -1;
    }

    *((uid_t *)output) = (uid_t)buffer;
    return 0;
}

int arg_gid(const char *input, void *output)
{
    /** Converts an input argument into a gid_t using arg_int64. Strictly
     * speaking this function only works up to 63 bits, but that is plenty
     * for everybody. */

    int64_t buffer;

    int result = arg_int64(input, &buffer);

    if ((result != 0) || (buffer < 0)) {
        return -1;
    }

    *((gid_t *)output) = (gid_t)buffer;
    return 0;
}
