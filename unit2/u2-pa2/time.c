#include <bits/types/struct_timeval.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "comm.h"

#define EXIT_BAD_PROCESS -2
#define EXIT_PROC_ERROR 3
#define EXIT_NOARGS -1
#define ERR_BAD_QUEUE 4

int run_executable(char *cmd, char **argv) {
    int result = execvp(cmd, argv);
    printf("The result of executable was: %d\n", result);
    return result;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "No executable provided\n");
        exit(EXIT_NOARGS);
    }
    struct timeval sys_start_time;
    gettimeofday(&sys_start_time, NULL);
    queue_t *messages = prepare_queue(1);
    pid_t exec_pid = fork();
    // Start the background process
    if (exec_pid == 0) {
        char* exec = argv[1];
        char** nargv = argv + 1;
        struct timeval usr_start_time;
        gettimeofday(&usr_start_time, NULL);
        queue_push(messages, &usr_start_time, sizeof(struct timeval));
        // Now run the exec
        if (run_executable(exec, nargv) == -1) {
            perror("An error ocurred while executing command: ");
            exit(EXIT_PROC_ERROR);
        }
        exit(0); // End here
    } else if (exec_pid < 0) {
        perror("Couldn't start process: ");
        exit(EXIT_BAD_PROCESS);
    }
    int status;
    wait(&status);
    struct timeval exec_stime;
    int popr = queue_pop(messages, &exec_stime);
    if (popr != 1) {
        fprintf(stderr, "An error ocurred, the queue was empty");
        exit(ERR_BAD_QUEUE);
    }
    queue_shutdown(messages);
    printf("The status of the child: %d\n", status);

    struct timeval sys_end_time;
    gettimeofday(&sys_end_time, NULL);
    printf("System time: %ldms\n", sys_end_time.tv_usec - sys_start_time.tv_usec);
}
