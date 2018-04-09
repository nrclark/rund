#ifndef _LIBSIGNAL_H_
#define _LIBSIGNAL_H_

/* Portable, POSIX-compliant implementation of a signal handler that queues
 * signals into pipes for later retrieval. This is a functionally similar to
 * the Linux-specific signalfd() interface, and also to the so-called
 * "self-pipe" technique. */

/*----------------------------------------------------------------------------*/

/* Connects a signal to a pipe so that it can be monitored with select()
 * or handled inside of a program's event loop.
 *
 * Returns the file-descriptor for the read-end of the pipe (this can also
 * be retrieved later with signal_pipefd_get(). */

int signal_pipefd_connect(int signum);

/* Returns the file descriptor for the read-end of the target signal's pipe,
 * for use with select() and friends. Signal-handler must be initialized with
 * signal_pipefd_connect() first.
 *
 * Note that when using the signal descriptor directly, be sure to clear the
 * received signal with read(fd, 1) or with signal_pipefd_clear(). */

int signal_pipefd_get(int signum);

/* Checks to see if a signal is waiting in the pipe. Returns -1 in the event
 * of an error, 0 if the check worked but no signal has arrives, and 1 if the
 * pipe has a pending signal. */

int signal_pipefd_check(int signum);

/* Clears a pending signal. Returns 0 on a success, or -1 otherwise. Trying to
 * clean an empty signal will result in a failure. */

int signal_pipefd_clear(int signum);

/* Blocks and waits for a signal to occur. Returns 0 on a success, -1 in the
 * event of an error. Automatically clears the signal. */

int signal_pipefd_wait(int signum);

/* Closes all signal-descriptors created by calls to signal_pipefd_connect().
 * Restores signal handlers to their default values. */

int signal_pipefd_cleanup(void);

#endif
