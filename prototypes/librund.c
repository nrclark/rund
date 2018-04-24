#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "libcommon.h"

extern char **environ;

static int find_program(const char *name, char *output)
{
    const char *path = getenv("PATH");

    unsigned int name_length = strnlen(name, NAME_MAX);
    unsigned int path_length;
    unsigned int start = 0;
    unsigned int end = 0;

    for (unsigned int x = 0; name[x] != '\x00'; x++) {
        if (name[x] == '/') {
            strncpy(output, name, PATH_MAX);
            output[PATH_MAX] = '\x00';
            return 0;
        }
    }

    if (path != NULL) {
        if (path[0] == '\x00') {
            path = NULL;
        }
    }

    if (path == NULL) {
        path = "/usr/bin:/bin";
    }

    while (1) {
        if ((path[end] == ':') || (path[end] == '\x00')) {
            path_length = end - start;

            if ((path_length + name_length + 1) > PATH_MAX) {
                path_length = PATH_MAX - (name_length + 1);
            }

            memcpy(output, path + start, path_length);
            output[path_length] = '/';
            memcpy(output + path_length + 1, name, name_length);
            output[name_length + path_length + 1] = '\x00';
            start = end + 1;

            if (access(output, R_OK | X_OK) == 0) {
                return 0;
            }
        }

        if (path[end] == '\x00') {
            break;
        }
        end++;
    }

    memcpy(output, "./", 2);
    memcpy(output + 2, name, name_length);
    output[2 + name_length] = '\x00';

    if (access(output, R_OK | X_OK) == 0) {
        return 0;
    }

    return -1;
}

int sleep_ms(unsigned int count)
{
    unsigned int seconds = (count / 1000);
    unsigned int msec = count - (seconds * 1000);
    long int nsec = msec * 1000000L;

    struct timespec duration = {
        .tv_sec = seconds,
        .tv_nsec = nsec
    };

    return nanosleep(&duration, NULL);
}

/*----------------------------------------------------------------------------*/

pid_t launch(char *const argv[], int stdin_fd, int stdout_fd, int stderr_fd)
{
    pid_t child;
    char filename[PATH_MAX + 1];
    int result;

    if (argv == NULL) {
        return -1;
    }

    if (argv[0] == NULL) {
        return -1;
    }

    result = find_program(argv[0], filename);

    if (result != 0) {
        fprintf(stderr, "error: couldn't launch [%s]: %s.\n", argv[0],
                strerror(errno));
        return result;
    }

    child = fork();

    if (child < 0) {
        return -1;
    }

    if (child == 0) {
        dup2(stdin_fd, STDIN_FILENO);
        dup2(stdout_fd, STDOUT_FILENO);
        dup2(stderr_fd, STDERR_FILENO);

        result = execve(filename, argv, environ);
        _exit(result);
    }

    return child;
}

int complete(pid_t process)
{
    int status;
    pid_t result;
    int options = WUNTRACED | WCONTINUED;

    while (1) {
        if (kill(process, 0) != 0) {
            fprintf(stderr, "error: couldn't talk to PID [%jd]: %s.\n",
                    (intmax_t) process, strerror(errno));
            return -1;
        }

        result = waitpid(process, &status, options);

        if (result < 0) {
            return result;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }

        kill(process, SIGCONT);
        sleep_ms(1);
    }
    return -1;
}

bool check_running(pid_t process, int8_t *errcode)
{
    int status;
    int options = WNOHANG;

    int result = waitpid(process, &status, options);

    if (result == -1) {
        perror(NULL);
        *errcode = -1;
        return false;
    }

    if (result == 0) {
        return true;
    }

    *errcode = (int8_t) (WEXITSTATUS(status));
    return !(WIFEXITED(status));
}

/*----------------------------------------------------------------------------*/

void register_handler(handler_t handler, int signum, bool block_others,
                      bool send_siginfo)
{
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

    sigaction(signum, &action, NULL);
}

/*----------------------------------------------------------------------------*/

static int child_pipe[2] = {-1, -1};

static void sigchild_handler(int signum, siginfo_t *info, void *ptr)
{
    (void) ptr;
    (void) info;
    (void) signum;

    if (write(child_pipe[1], "\x00", 1) < 0) {
        perror(NULL);
    }
}

static void sigint_handler(int signum, siginfo_t *info, void *ptr)
{
    (void) ptr;
    (void) info;
    (void) signum;
    fprintf(stderr, "gooby %jd\n", (intmax_t) getpid());
}

void libcommon_init(void)
{
    if (child_pipe[0] != -1) {
        return;
    }

    if (pipe(child_pipe)) {
        perror(NULL);
        exit(0);
    }

    if (fcntl(child_pipe[0], F_SETFL, FD_CLOEXEC)) {
        perror(NULL);
        exit(0);
    }

    if (fcntl(child_pipe[1], F_SETFL, FD_CLOEXEC)) {
        perror(NULL);
        exit(0);
    }

    register_handler(sigchild_handler, SIGCHLD, true, false);
}

void libcommon_wait(void)
{
    char dummy_buffer;
    read(child_pipe[0], &dummy_buffer, 1);
}

pid_t libcommon_launch(char *const argv[], int stdin_fd, int stdout_fd,
                     int stderr_fd)
{
    pid_t result = launch(argv, stdin_fd, stdout_fd, stderr_fd);
    return result;
}
