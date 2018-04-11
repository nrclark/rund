#ifndef _LIBNOINTR_H_
#define _LIBNOINTR_H_

#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int select_nointr(int nfds, fd_set *restrict readfds, fd_set *restrict writefds,
                  fd_set *restrict errorfds, struct timeval *restrict timeout);

ssize_t read_nointr(int fd, void *buf, size_t nbytes);
ssize_t write_nointr(int fd, const void *buf, size_t nbytes);

int open_nointr(const char *path, int oflag);
int close_nointr(int fildes);

FILE * fopen_nointr(const char *restrict pathname, const char *restrict mode);
int fclose_nointr(FILE *stream);

pid_t waitpid_nointr(pid_t pid, int *stat_loc, int options);

#endif
