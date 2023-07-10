#include "bmp.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "filter.h"

#ifdef KITTY_IMAGE_OUTPUT
#include <sys/wait.h>
#include <unistd.h>
void previewImage(char *src) {
    pid_t kitty_pid = fork();
    if (kitty_pid == 0) {
        char *args[] = {"/usr/bin/kitty", "+kitten", "icat", src, '\0'};
        execvp("/usr/bin/kitty", args);
        exit(-9991);
    }
    int ret = 0;
    wait(&ret);
    printf("Kitty returned: %d\n", ret);
}
#endif

sem_t plock;

int main(int argc, char **argv) {
    FILE *source;
    FILE *dest;
    BMP_Image *image = NULL;

    if (argc != 3) {
        printError(ARGUMENT_ERROR);
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));
    sem_init(&plock, 0, 1);

    if ((source = fopen(argv[1], "rb")) == NULL) {
        printError(FILE_ERROR);
        exit(EXIT_FAILURE);
    }
    if ((dest = fopen(argv[2], "wb")) == NULL) {
        printError(FILE_ERROR);
        exit(EXIT_FAILURE);
    }
#ifdef KITTY_IMAGE_OUTPUT
    previewImage(argv[1]);
#endif

    image = createBMPImage(source);
    printBMPHeader(&image->header);
    //   readImage(source, image);

#ifdef DEBUG_POINTERS
    printf("[DEBUG] Pointer to readed image: %p\n", image);
#endif

    if (!checkBMPValid(&image->header)) {
        printError(VALID_ERROR);
        exit(EXIT_FAILURE);
    }
    rewind(source);
    BMP_Image *imgdest = createBMPImage(source);
    //   readImage(source, image);
    printBMPHeader(&image->header);
    printBMPImage(image);
    apply(image, imgdest);
    writeImage(argv[2], imgdest);
    freeImage(image);
    
    fclose(source);
    fclose(dest);
#ifdef KITTY_IMAGE_OUTPUT
    printf("The output is: \n");
    previewImage(argv[2]);
#endif

    exit(EXIT_SUCCESS);
}
