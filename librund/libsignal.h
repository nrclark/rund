#ifndef _LIBSIGNAL_H_
#define _LIBSIGNAL_H_

void signal_pipefd_init(void);

int signal_pipefd_listen(int signum);

int signal_pipefd_get(int signum);
int signal_pipefd_ack(int signum);

int signal_pipefd_wait(int signum);

#endif
