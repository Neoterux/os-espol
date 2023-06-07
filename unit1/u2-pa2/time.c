#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#define EXIT_BAD_PROCESS -2
#define EXIT_PROC_ERROR 3
#define EXIT_NOARGS -1

inline void* next_address(void *pointer, size_t size) {
    return pointer + size;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "No executable provided\n");
        exit(EXIT_NOARGS);
    }
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    pid_t exec_pid = fork();
    // Start the background process
    if (exec_pid == 0) {
        char* executable = argv[1];
        int result = execv(executable, argv);
        if (result == -1) {
            perror("An error ocurred while executing command: ");
            exit(EXIT_PROC_ERROR);
        }
        exit(0); // End here
    } else if (exec_pid < 0) {
        perror("Couldn't start process: ");
        exit(EXIT_BAD_PROCESS);
    }

    struct timeval end_time;
    gettimeofday(&end_time, NULL);
}
