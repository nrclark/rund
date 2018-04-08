#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

#include "libsignal.h"

/*----------------------------------------------------------------------------*/

enum {max_signum = 31};

static int pipe_set[max_signum + 1][2];
static bool initialized = false;

/*----------------------------------------------------------------------------*/

typedef void (*handler_t)(int signum, siginfo_t *info, void *ptr);

static int register_handler(handler_t handler, int signum, bool block_others,
                            bool send_siginfo)
{
    int result;

    struct sigaction action = {
        .sa_sigaction = handler,
        .sa_flags = SA_RESTART
    };

    if (send_siginfo) {
        action.sa_flags |= SA_SIGINFO;
    }

    if (block_others) {
        sigfillset(&action.sa_mask);
    } else {
        sigemptyset(&action.sa_mask);
    }

    result = sigaction(signum, &action, NULL);

    if (result != 0) {
        perror("Couldn't register handler");
    }

    return result;
}

static int fd_set_flags(int fd, int flags)
{
    int result;
    errno = 0;

    result = fcntl(fd, F_GETFL);

    if ((errno != 0) || (result < 0)) {
        perror("couldn't read fd flags");
        return -1;
    }

    if (fcntl(fd, F_SETFL, result | flags)) {
        perror("couldn't set fd flags");
        return -1;
    }
    return 0;
}

/* returns 0 if the fd is ready for reading. */
static int fd_check_ready(int fd)
{
    fd_set read_fds;
    fd_set error_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&error_fds);
    FD_SET(fd, &read_fds);
    FD_SET(fd, &error_fds);

    int result = select(fd + 1, &read_fds, NULL, &error_fds, NULL);

    if (result < 0) {
        perror("select call failed");
        return result;
    }

    /* We might normally use FD_ISSET here, but this isn't necessary
     * because we're only listening for one item (the socket). */

    if (result == 0) {
        return 1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/

static void pipefd_handler(int signum, siginfo_t *info, void *ptr)
{
    (void) ptr;
    (void) info;

    if (write(pipe_set[signum][1], "\x00", 1) < 0) {
        perror("Couldn't write to notification pipe");
    }
}
/*----------------------------------------------------------------------------*/

void signal_pipefd_init(void)
{
    initialized = true;

    for (unsigned int x = 0; x < max_signum; x++) {
        pipe_set[x][0] = -1;
        pipe_set[x][1] = -1;
    }
}

int signal_pipefd_listen(int signum)
{
    int result;

    if (signum > max_signum) {
        fprintf(stderr, "Requested signal too big!\n");
        return -1;
    }

    if (initialized == false) {
        signal_pipefd_init();
    }

    if (pipe_set[signum][0] != -1) {
        return pipe_set[signum][0];
    }

    if (pipe(pipe_set[signum])) {
        perror("Couldn't open file descriptors for pipe");
        return -1;
    }

    if (fd_set_flags(pipe_set[signum][0], FD_CLOEXEC)) {
        return -1;
    }

    if (fd_set_flags(pipe_set[signum][1], FD_CLOEXEC | O_NONBLOCK)) {
        return -1;
    }

    result = register_handler(pipefd_handler, signum, true, false);

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

    return pipe_set[signum][0];
}

int signal_pipefd_ack(int signum)
{
    int result;
    char dummy_buffer;

    if (signum > max_signum) {
        fprintf(stderr, "Requested signal too big!\n");
        return -1;
    }

    result = fd_check_ready(pipe_set[signum][0]);

    if (result < 0) {
        return result;
    }

    if (result == 0) {
        result = read(pipe_set[signum][0], &dummy_buffer, 1);

        if (result < 0) {
            perror("read from pipe failed");
        }

        return (result == 1);
    }

    return result;
}

int signal_pipefd_wait(int signum)
{
    int result;
    char dummy_buffer;

    if (signum > max_signum) {
        fprintf(stderr, "Requested signal too big!\n");
        return -1;
    }

    result = read(pipe_set[signum][0], &dummy_buffer, 1);

    if (result < 0) {
        perror("read from pipe failed");
    }

    return (result == 1);
}
