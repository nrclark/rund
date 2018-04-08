#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "libproc.h"
#include "libpath.h"

extern char **environ;

static int sleep_ms(unsigned int count)
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

pid_t proc_launch(char *const argv[], int stdin_fd, int stdout_fd,
                  int stderr_fd)
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

    result = path_findprog(argv[0], filename);

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

int8_t proc_polled_wait(pid_t process)
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

bool proc_running(pid_t process, int8_t *errcode)
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
