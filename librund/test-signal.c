#include "config.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "libsignal.h"

#define sigdef_line(x) .signum = (x), .fd = -1, .name = #x

struct sigdef {
    int signum;
    int fd;
    const char *name;
};

/* This is the full set of POSIX signals at the time of this writing (POSIX
 * 2008). Your OS might other signals that aren't listed here. The last line
 * is a terminator. */

struct sigdef signals[] = {
    {sigdef_line(SIGABRT)},
    {sigdef_line(SIGALRM)},
    {sigdef_line(SIGBUS)},
    {sigdef_line(SIGCHLD)},
    {sigdef_line(SIGCONT)},
    {sigdef_line(SIGFPE)},
    {sigdef_line(SIGHUP)},
    {sigdef_line(SIGILL)},
    {sigdef_line(SIGINT)},
    {sigdef_line(SIGKILL)},
    {sigdef_line(SIGPIPE)},
    {sigdef_line(SIGQUIT)},
    {sigdef_line(SIGSEGV)},
    {sigdef_line(SIGSTOP)},
    {sigdef_line(SIGTERM)},
    {sigdef_line(SIGTSTP)},
    {sigdef_line(SIGTTIN)},
    {sigdef_line(SIGTTOU)},
    {sigdef_line(SIGUSR1)},
    {sigdef_line(SIGUSR2)},
    {sigdef_line(SIGPOLL)},
    {sigdef_line(SIGPROF)},
    {sigdef_line(SIGSYS)},
    {sigdef_line(SIGTRAP)},
    {sigdef_line(SIGURG)},
    {sigdef_line(SIGVTALRM)},
    {sigdef_line(SIGXCPU)},
    {sigdef_line(SIGXFSZ)},
    {.signum = -1, .name = NULL},
};

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

static int lookup_signal(const char *name)
{
    for (unsigned int x = 0; signals[x].signum != -1; x++) {
        if (strcmp(name, signals[x].name) == 0) {
            return (int) x;
        }
    }
    fprintf(stderr, "error: couldn't lookup signal [%s]\n", name);
    return -1;
}

int main(int argc, char *argv[])
{
    int result;
    int count = 0;
    int ticks = 0;
    int sec = 0;
    int target_signal = SIGUSR1;

    if (argc > 2) {
        target_signal = lookup_signal(argv[1]);
    }

    printf("Process is PID %ju\n\n", (uintmax_t)(getpid()));

    signal_pipefd_connect(target_signal);

    printf("Testing polled interface\n");
    while (1) {
        sleep_ms(5);
        ticks++;

        if (ticks >= 200) {
            ticks = 0;
            sec++;
            printf("Seconds: %d\n", sec);
        }

        result = signal_pipefd_check(target_signal);

        if (result > 0) {
            count++;
            printf("Got signal!\n");
            signal_pipefd_clear(target_signal);
        } else if (count > 0) {
            break;
        }
    }

    printf("Entering main-loop\n");

    while (1) {
        signal_pipefd_wait(target_signal);
        printf("Got signal!\n");
    }

    signal_pipefd_cleanup();
    return 0;
}
