#ifndef _LIBNOINTR_H_
#define _LIBNOINTR_H_

#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
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

int accept_nointr(int socket, struct sockaddr *restrict address,
                  socklen_t *restrict address_len);

int connect_nointr(int socket, const struct sockaddr *address,
                   socklen_t address_len);

int dup2_nointr(int fildes, int fildes2);

int nanosleep_nointr(const struct timespec *rqtp, struct timespec *rmtp);

#endif
