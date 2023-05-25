#include <stddef.h>
#include <unistd.h>

#define STDOUT 0
#define BUFF_SIZE 256
typedef char* string;

void printd(int descriptor, string message) {
    if (message == NULL) return;
    char buffer[BUFF_SIZE];
    char c;
    string pointer = message;
    size_t bufferpoint = 0;
    while((c = *pointer++) != 0) bufferpoint++;
    write(descriptor, message, bufferpoint);
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

void readline(int fd, string out, size_t buffsize) {

}


int main(int argc, char **args) {
    if (argc > 2) {
        print("Use: mycat namefile.\n");
        return 0;
    }
    print("Hola mundo");
}
