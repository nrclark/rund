#ifndef _LIBCOMMON_H_
#define _LIBCOMMON_H_

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef void (*handler_t)(int signum, siginfo_t *info, void *ptr);

void register_handler(handler_t handler, int signum, bool block_others,
                      bool send_siginfo);

pid_t launch(char *const argv[], int stdin_fd, int stdout_fd, int stderr_fd);
int complete(pid_t process);
bool check_running(pid_t process, int8_t *errcode);

/*----------------------------------------------------------------------------*/

int sleep_ms(unsigned int count);

void libcommon_init(void);
pid_t libcommon_launch(char *const argv[], int stdin_fd, int stdout_fd,
                     int stderr_fd);
void libcommon_wait(void);

#endif
