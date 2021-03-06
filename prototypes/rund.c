#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "libcommon.h"

int main(int argc, char *argv[], char *envp[])
{
    (void) argc;
    (void) argv;
    (void) envp;

    printf("I am %lu\n", (unsigned long)getpid());
    char *const cmd[] = {(char *)"./test.sh", NULL};
    libcommon_init();

    pid_t result = libcommon_launch(cmd, STDIN_FILENO, STDOUT_FILENO,
                                  STDERR_FILENO);
    int8_t errcode;
    while (check_running(result, &errcode)) {}

    printf("Errcode: %d\n", errcode);
    check_running(345309450, &errcode);

    //libcommon_wait();
    printf("Child complete\n");
    printf("Errcode: %d\n", errcode);
    return 0;
}
