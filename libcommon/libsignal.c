#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

#include "libnointr.h"
#include "libsignal.h"

/*----------------------------------------------------------------------------*/

enum {max_signum = 31};

static int pipe_set[max_signum + 1][2];
static bool initialized = false;

/*----------------------------------------------------------------------------*/

typedef void (*handler_t)(int signum);

static int register_handler(handler_t handler, int signum, bool block_others)
{
    int result;

    struct sigaction action = {
        .sa_handler = handler,
        .sa_flags = SA_RESTART
    };

    if (block_others) {
        sigfillset(&action.sa_mask);
    } else {
        sigemptyset(&action.sa_mask);
    }
    action.sa_flags = SA_RESTART;
    result = sigaction(signum, &action, NULL);

    if (result != 0) {
        perror("Couldn't register handler");
    }

    return result;
}

static int fd_set_flags(int fd, int getcmd, int setcmd, int flags)
{
    int result;
    errno = 0;

    result = fcntl(fd, getcmd);

    if ((errno != 0) || (result < 0)) {
        perror("couldn't read fd flags");
        return -1;
    }

    if (fcntl(fd, setcmd, result | flags)) {
        perror("couldn't set fd flags");
        return -1;
    }
    return 0;
}

static int fd_set_flflags(int fd, int flags)
{
    return fd_set_flags(fd, F_GETFL, F_SETFL, flags);
}

static int fd_set_fdflags(int fd, int flags)
{
    return fd_set_flags(fd, F_GETFD, F_SETFD, flags);
}

/* returns -1 in an error, 0 if the fd is blocked, and 1 if the fd is
 * ready for reading. */
static int fd_check_ready(int fd)
{
    fd_set read_fds;
    fd_set error_fds;
    struct timeval timeout = {0};

    FD_ZERO(&read_fds);
    FD_ZERO(&error_fds);
    FD_SET(fd, &read_fds);
    FD_SET(fd, &error_fds);

    int result = select_nointr(fd + 1, &read_fds, NULL, &error_fds, &timeout);

    if (result < 0) {
        perror("select call failed");
        return result;
    }

    /* We might normally use FD_ISSET here, but this isn't necessary
     * because we're only listening for one item (the socket). */

    return (result == 0) ? 0 : 1;
}

static void pipe_set_init(void)
{
    initialized = true;

    for (unsigned int x = 0; x < max_signum; x++) {
        pipe_set[x][0] = -1;
        pipe_set[x][1] = -1;
    }
}

/*----------------------------------------------------------------------------*/

static void pipefd_handler(int signum)
{
    if (write_nointr(pipe_set[signum][1], "\x00", 1) < 0) {
        if (errno != EAGAIN) {
            perror("Couldn't write to notification pipe");
        }
    }
}

/*----------------------------------------------------------------------------*/

int signal_pipefd_connect(int signum)
{
    int result;

    if (signum > max_signum) {
        fprintf(stderr, "Requested signal too big!\n");
        return -1;
    }

    if (initialized == false) {
        pipe_set_init();
    }

    if (pipe_set[signum][0] != -1) {
        return pipe_set[signum][0];
    }

    if (pipe(pipe_set[signum])) {
        perror("Couldn't open file descriptors for pipe");
        return -1;
    }

    if (fd_set_fdflags(pipe_set[signum][0], FD_CLOEXEC)) {
        return -1;
    }

    if (fd_set_fdflags(pipe_set[signum][1], FD_CLOEXEC)) {
        return -1;
    }

    if (fd_set_flflags(pipe_set[signum][1], O_NONBLOCK)) {
        return -1;
    }

    result = register_handler(pipefd_handler, signum, true);

    if (result != 0) {
        return result;
    }

    return pipe_set[signum][0];
}

int signal_pipefd_get(int signum)
{
    if (signum > max_signum) {
        fprintf(stderr, "Requested signal too big!\n");
        return -1;
    }

    if (initialized == false) {
        pipe_set_init();
    }

    return pipe_set[signum][0];
}

int signal_pipefd_clear(int signum)
{
    int result;
    char dummy_buffer;

    if (signum > max_signum) {
        fprintf(stderr, "Requested signal too big!\n");
        return -1;
    }

    if (initialized == false) {
        pipe_set_init();
    }

    result = fd_check_ready(pipe_set[signum][0]);

    if (result < 0) {
        return result;
    }

    if (result == 1) {
        result = read_nointr(pipe_set[signum][0], &dummy_buffer, 1);

        if (result < 0) {
            perror("read from pipe failed");
            return result;
        }

        if (result == 0) {
            fprintf(stderr, "warning: read from pipefd was empty\n");
            return -1;
        }

        return 0;
    }

    fprintf(stderr, "warning: tried to clear empty signal\n");
    return -1;
}

int signal_pipefd_check(int signum)
{
    if (signum > max_signum) {
        fprintf(stderr, "Requested signal too big!\n");
        return -1;
    }

    if (initialized == false) {
        pipe_set_init();
    }

    return fd_check_ready(pipe_set[signum][0]);
}

int signal_pipefd_wait(int signum)
{
    int result;
    char dummy_buffer;

    if (signum > max_signum) {
        fprintf(stderr, "Requested signal too big!\n");
        return -1;
    }

    if (initialized == false) {
        pipe_set_init();
    }

    result = read_nointr(pipe_set[signum][0], &dummy_buffer, 1);

    if (result < 0) {
        perror("read from pipe failed");
        return result;
    }

    if (result == 0) {
        fprintf(stderr, "warning: read from pipefd was empty\n");
        return -1;
    }

    return 0;
}

int signal_pipefd_cleanup(void)
{
    int result;

    if (initialized == false) {
        return 0;
    }

    for (unsigned int signum = 0; signum < max_signum; signum++) {
        if (pipe_set[signum][0] != -1) {
            signal((int) signum, SIG_DFL);
        }

        for (unsigned int x = 0; x < 2; x++) {
            if (pipe_set[signum][x] != -1) {
                result = close_nointr(pipe_set[signum][x]);

                if (result != 0) {
                    perror("couldn't close pipefd");
                    return result;
                }

                pipe_set[signum][x] = -1;
            }
        }
    }

    initialized = false;
    return 0;
}
