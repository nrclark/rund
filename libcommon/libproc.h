#ifndef _LIBPROC_H_
#define _LIBPROC_H_

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

pid_t proc_launch(char *const argv[], int stdin_fd, int stdout_fd,
                  int stderr_fd);

int8_t proc_polled_wait(pid_t process);

bool proc_running(pid_t process, int8_t *errcode);

#endif
