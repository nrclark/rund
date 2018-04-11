#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "libnointr.h"

int select_nointr(int nfds, fd_set *restrict readfds, fd_set *restrict writefds,
                  fd_set *restrict errorfds, struct timeval *restrict timeout)
{
    int result;

    while (1) {
        result = select(nfds, readfds, writefds, errorfds, timeout);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

ssize_t read_nointr(int fd, void *buf, size_t nbytes)
{
    ssize_t result;

    while (1) {
        result = read(fd, buf, nbytes);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

ssize_t write_nointr(int fd, const void *buf, size_t nbytes)
{
    ssize_t result;

    while (1) {
        result = write(fd, buf, nbytes);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

int close_nointr(int fildes)
{
    int result;

    while (1) {
        result = close(fildes);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

int open_nointr(const char *path, int oflag)
{
    int result;

    while (1) {
        result = open(path, oflag);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

FILE * fopen_nointr(const char *restrict pathname, const char *restrict mode)
{
    FILE *result;

    while (1) {
        result = fopen(pathname, mode);

        if ((result == NULL) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

int fclose_nointr(FILE *stream)
{
    int result;

    while (1) {
        result = fclose(stream);

        if ((result != 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

pid_t waitpid_nointr(pid_t pid, int *stat_loc, int options)
{
    pid_t result;

    while (1) {
        result = waitpid(pid, stat_loc, options);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

int accept_nointr(int socket, struct sockaddr *restrict address,
                  socklen_t *restrict address_len)
{
    int result;

    while (1) {
        result = accept(socket, address, address_len);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

int connect_nointr(int socket, const struct sockaddr *address,
                   socklen_t address_len)
{
    int result;

    while (1) {
        result = connect(socket, address, address_len);

        if ((result != 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

int dup2_nointr(int fildes, int fildes2)
{
    int result;

    while (1) {
        result = dup2(fildes, fildes2);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}

int nanosleep_nointr(const struct timespec *rqtp, struct timespec *rmtp)
{
    int result;

    while (1) {
        result = nanosleep(rqtp, rmtp);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        return result;
    }
}
