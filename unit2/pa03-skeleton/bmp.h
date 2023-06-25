#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#ifndef _BMP_H_
#define _BMP_H_
#define TRUE 1
#define FALSE 0
#define ARGUMENT_ERROR  0x01
#define FILE_ERROR      0x02
#define MEMORY_ERROR    0x03
#define VALID_ERROR     0x04
#define INVALID_FILE    0x05
#define HEADER_SIZE 54


// Set data alignment to 1 byte boundary
#pragma pack(1)

#define BM_COMPRGB  0x00
#define BM_COMPRLE8 0x01
#define BM_COMPRLE4 0x02
#define BM_COMPBTFD 0x03 
#define BM_COMPJPEG 0x04
#define BM_COMPPNG  0x05
#define BM_COMPALBF 0x06 // Alpha Bit Fields
#define BM_COMPCMYK 0x0b // CMYK
#define BM_COMPCMR8 0x0c // CMYK RLE 8
#define BM_COMPCMR4 0x0d // CMYK RLE4

/**
 * Bitmap header types
 */
typedef enum {
    BM_BITMAPCOREHEADER = 0,
    BM_OS21XBITMAPHEADER,
    BM_OS22XBITMAPHEADER,
    BM_BITMAPINFOHEADER,
    BM_BITMAPV2INFOHEADER,
    BM_BITMAPV3INFOHEADER,
    BM_BITMAPV4INFOHEADER,
    BM_BITMAPV5INFOHEADER
} bitmap_header_t;



/*
 * BMP files are laid out in the following fashion:
 *   --------------------------
 *   |          Header        |   54 bytes
 *   |-------------------------
 *   |    Palette (optional)  |   0 bytes (for 24-bit RGB images)
 *   |-------------------------
 *   |       Image Data       |   file size - 54 (for 24-bit images)
 *   --------------------------
 */

/** 
 * BMP header (54 bytes).
 */

typedef struct __attribute__((packed)) BMP_Header {
    uint16_t type;           // Magic identifier
    uint32_t size;           // File size in bytes
    uint16_t reserved1;      // Not used
    uint16_t reserved2;      // Not used
    uint32_t offset;         // Offset to image data in bytes
    /* BITMAPV5HEADER|BITMAPINFOHEADER */
    uint32_t header_size;    // Header size in bytes
    int32_t  width_px;       // Width of the image
    int32_t  height_px;      // Height of image
    uint16_t planes;                    // Number of color planes
    uint16_t bits_per_pixel;            // Bits per pixel
    uint32_t compression;               // Compression type
    uint32_t imagesize;                 // Image size in bytes
    int32_t  xresolution;               // Pixels per meter
    int32_t  yresolution;               // Pixels per meter
    uint32_t ncolours;                  // Number of colors  
    uint32_t importantcolours;          // Important colors
} BMP_Header;

typedef struct __attribute__((packed)) Pixel {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} Pixel;

typedef struct BMP_Image {
    BMP_Header header;
    int norm_height; //normalized height
    int bytes_per_pixel; // This amount should be equals to number of bits/8
    Pixel ** pixels;
} BMP_Image;

void printError(int error);
BMP_Image* createBMPImage();
void readImageData(FILE *srcFile, BMP_Image *dataImage, int dataSize);
void readImage(FILE *srcFile, BMP_Image *dataImage);
void writeImage(char* destFileName, BMP_Image* dataImage);
void freeImage(BMP_Image* image);
int checkBMPValid(BMP_Header* header);
void printBMPHeader(BMP_Header* header);
void printBMPImage(BMP_Image* image);

long getCPUCount();

#endif /* bmp.h */
