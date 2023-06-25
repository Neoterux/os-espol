#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "bmp.h"

#define MIN_TWORKERS 4
#define max(a, b) (a > b ? a : b)
/* USE THIS FUNCTION TO PRINT ERROR MESSAGES
   DO NOT MODIFY THIS FUNCTION
*/

void printError(int error){
  switch(error){
  case ARGUMENT_ERROR:
    printf("Usage:ex5 <source> <destination>\n");
    break;
  case FILE_ERROR:
    printf("Unable to open file!\n");
    break;
  case MEMORY_ERROR:
    printf("Unable to allocate memory!\n");
    break;
  case VALID_ERROR:
    printf("BMP file not valid!\n");
    break;
  default:
    break;
  }
}

#define BMP_HSIZE sizeof(BMP_Header)

/* The input argument is the source file pointer. The function will first construct a BMP_Image image by allocating memory to it.
 * Then the function read the header from source image to the image's header.
 * Compute data size, width, height, and bytes_per_pixel of the image and stores them as image's attributes.
 * Finally, allocate menory for image's data according to the image size.
 * Return image;
*/
BMP_Image* createBMPImage(FILE* fptr) {

    //Allocate memory for BMP_Image*;
    BMP_Image *image = malloc(sizeof(BMP_Image));

    //Read the first 54 bytes of the source into the header
    int readbytes = fread(&image->header, BMP_HSIZE, 1, fptr);
    if (readbytes != 1) {
        printError(VALID_ERROR);
        exit(VALID_ERROR);
    }

    //Compute data size, width, height, and bytes per pixel;
    image->bytes_per_pixel = image->header.bits_per_pixel/8;
    const int CPU_COUNT = max(getCPUCount() - 1, MIN_TWORKERS);
    image->norm_height = ((int) ceil((float)image->header.height_px / (float)CPU_COUNT)) * CPU_COUNT;

    //Allocate memory for image data
    const int ROW_COUNT = image->norm_height;
    const int ROW_SIZE = image->header.width_px;
    Pixel *content_buffer = calloc(ROW_SIZE * ROW_COUNT, sizeof(Pixel));
    Pixel **matrix = malloc(sizeof(Pixel*) * ROW_COUNT);
    image->pixels = matrix;
    
    Pixel buffer[ROW_SIZE];
    const int ROWS_BYTES = ROW_SIZE * sizeof(Pixel);
    for(int row = 0; row < ROW_COUNT; row++) {
        Pixel *pixel_row = content_buffer + ROW_SIZE * row;
        matrix[row] = pixel_row;
        int r = fread(buffer, sizeof(Pixel), ROW_SIZE, fptr);
        if (r)
            memcpy(pixel_row, buffer, ROWS_BYTES);
    }

    return image;
}

/* The input arguments are the source file pointer, the image data pointer, and the size of image data.
 * The functions reads data from the source into the image data matriz of pixels.
*/
void readImageData(FILE* srcFile, BMP_Image * image, int dataSize) {

}

/* The input arguments are the pointer of the binary file, and the image data pointer.
 * The functions open the source file and call to CreateBMPImage to load de data image.
*/
void readImage(FILE *srcFile, BMP_Image * dataImage) {
    dataImage = createBMPImage(srcFile);
    printBMPHeader(&dataImage->header);
    exit(0);
}

/* The input arguments are the destination file name, and BMP_Image pointer.
 * The function write the header and image data into the destination file.
*/
void writeImage(char* destFileName, BMP_Image* dataImage) {
}

/* The input argument is the BMP_Image pointer. The function frees memory of the BMP_Image.
*/
void freeImage(BMP_Image* image) {
}

/* The functions checks if the source image has a valid format.
 * It returns TRUE if the image is valid, and returns FASLE if the image is not valid.
 * DO NOT MODIFY THIS FUNCTION
*/
int checkBMPValid(BMP_Header* header) {
  // Make sure this is a BMP file
  if (header->type != 0x4d42) {
    return FALSE;
  }
  // Make sure we are getting 24 bits per pixel
  if (header->bits_per_pixel != 24) {
    return FALSE;
  }
  // Make sure there is only one image plane
  if (header->planes != 1) {
    return FALSE;
  }
  // Make sure there is no compression
  if (header->compression != 0) {
    return FALSE;
  }
  return TRUE;
}

/* The function prints all information of the BMP_Header.
   DO NOT MODIFY THIS FUNCTION
*/
void printBMPHeader(BMP_Header* header) {
  printf("file type (should be 0x4d42): %x\n", header->type);
  printf("file size: %d\n", header->size);
  printf("offset to image data: %d\n", header->offset);
  printf("header size: %d\n", header->header_size);
  printf("width_px: %d\n", header->width_px);
  printf("height_px: %d\n", header->height_px);
  printf("planes: %d\n", header->planes);
  printf("bits: %d\n", header->bits_per_pixel);
}

/* The function prints information of the BMP_Image.
   DO NOT MODIFY THIS FUNCTION
*/
void printBMPImage(BMP_Image* image) {
  printf("data size is %ld\n", sizeof(image->pixels));
  printf("norm_height size is %d\n", image->norm_height);
  printf("bytes per pixel is %d\n", image->bytes_per_pixel);
}


long getCPUCount() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}
