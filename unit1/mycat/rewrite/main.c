#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#define STDOUT 0
#define STDIN 1
#define BUFF_SIZE 256
typedef char* string;

/**
* Write the given string into a file by it's file
* decriptor.
*/
void printd(int descriptor, string message) {
    if (message == NULL) return;
    // char buffer[BUFF_SIZE];
    char c;
    string pointer = message;
    size_t bufflen = 0;
    while((c = *pointer++)) bufflen++;
    write(descriptor, message, bufflen);
    // while (bufferpoint < BUFF_SIZE && (c = *message++) != 0) {
    //     buffer[bufferpoint++] = c;
    // }
}

/**
*   Print a message to the stdout.
*/
void print(string message) {
    printd(STDOUT, message);
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        print("Use: mycat namefile.\n");
        return 0;
    }
    int inputfd = STDIN;
    if (argc != 1 && (inputfd = open(argv[1], O_RDONLY)) < 0) {
        perror("An error ocurred while open file");
        return errno;
    }
    int readres;
    char filebuffer[BUFF_SIZE];
    while ((readres = read(inputfd, filebuffer, BUFF_SIZE - 1))) {
        // Ensure this is a zero terminated string (prevent buffer overflow)
        filebuffer[readres] = 0;
        print(filebuffer);
    }
    return 0;
}
