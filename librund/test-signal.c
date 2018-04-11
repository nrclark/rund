#include "config.h"

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
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

static const char * lookup_sig_name(int signum)
{
    static const char empty[1] = "\x00";

    for (unsigned int x = 0; signals[x].signum != -1; x++) {
        if (signals[x].signum == signum) {
            return signals[x].name;
        }
    }

    fprintf(stderr, "error: couldn't lookup signal [%d]\n", signum);
    return empty;
}

static int wait_signals(int signal_list[], unsigned int count)
{
    fd_set read_fds;
    fd_set error_fds;
    int fd_list[count];
    int max_fd = -1;

    FD_ZERO(&read_fds);
    FD_ZERO(&error_fds);

    for (unsigned int x = 0; x < count; x++) {
        fd_list[x] = signal_pipefd_get(signal_list[x]);
        FD_SET(fd_list[x], &read_fds);
        FD_SET(fd_list[x], &error_fds);

        if (fd_list[x] > max_fd) {
            max_fd = fd_list[x];
        }
    }

    while (1) {
        int result = select(max_fd + 1, &read_fds, NULL, &error_fds, NULL);

        if ((result < 0) && (errno == EINTR)) {
            continue;
        }

        if (result < 0) {
            perror("select call failed");
            return result;
        }

        break;
    }

    for (int x = 0; x < (int) count; x++) {
        if (FD_ISSET(fd_list[x], &error_fds)) {
            fprintf(stderr, "wait_signals got an error on signal[%d]\n", x);
            signal_pipefd_clear(signal_list[x]);
        }

        if (FD_ISSET(fd_list[x], &read_fds)) {
            fprintf(stderr, "wait_signals received signal [%s]\n",
                    lookup_sig_name(signal_list[x]));
            signal_pipefd_clear(signal_list[x]);
        }
    }

    return 0;
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
    printf("Testing polled interface\n");

    signal_pipefd_connect(SIGUSR1);

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

    signal_pipefd_connect(SIGUSR2);

    printf("Entering main-loop\n");
    int signal_set[2] = {SIGUSR1, SIGUSR2};
    while (1) {
        wait_signals(signal_set, 2);
    }

    signal_pipefd_cleanup();
    return 0;
}
