#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"


#ifdef KITTY_IMAGE_OUTPUT
#include <unistd.h>
#include <sys/wait.h>
void previewImage(char* src) {
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

int main(int argc, char **argv) {
  FILE* source;
  FILE* dest;
  BMP_Image* image = NULL;

  if (argc != 3) {
    printError(ARGUMENT_ERROR);
    exit(EXIT_FAILURE);
  }
  
  if((source = fopen(argv[1], "rb")) == NULL) {
    printError(FILE_ERROR);
    exit(EXIT_FAILURE);
  }
  if((dest = fopen(argv[2], "wb")) == NULL) {
    printError(FILE_ERROR);
    exit(EXIT_FAILURE);
  } 
#ifdef KITTY_IMAGE_OUTPUT
    previewImage(argv[1]);
#endif  

  readImage(source, image);

  if(!checkBMPValid(&image->header)) {
    printError(VALID_ERROR);
    exit(EXIT_FAILURE);
  }



  readImage(source, image);
  printBMPHeader(&image->header);
  printBMPImage(image);

  freeImage(image);
  fclose(source);
  fclose(dest);

  exit(EXIT_SUCCESS);
}
