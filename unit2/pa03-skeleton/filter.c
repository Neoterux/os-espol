#include "bmp.h"
#include <bits/pthreadtypes.h>
#include <stddef.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)
#define abs(a) (a < 0 ? -a : a)
#define diff(a, b) abs(a - b)
extern sem_t plock;

typedef enum {
    BLUE = 1,
    GREEN,
    RED,
    ALPHA
} channel_t;

typedef struct {
    BMP_Image *src;
    BMP_Image *dest;
    int init_row;
    int init_col;
    size_t row_count;
    double factor;
#ifdef DEBUG_VERBOSE
    unsigned char skip;
#endif
} worker_arg_t;

int thread_serial_count = 0;
sem_t serialsem;

int acquire_id() {
    sem_wait(&serialsem);
    int id = ++thread_serial_count;
    sem_post(&serialsem);
    return id;
}

void *filterThreadWorker(void * args);

void tprintf(char *format, ...) {
    sem_wait(&plock);
    pthread_t tid = pthread_self();
    printf("[t=%ld]", tid);
    va_list argp;
    va_start(argp, format);
    vprintf(format, argp);
    va_end(argp);

    sem_post(&plock);
}

void print_matrix(int matrix[3][3]) {
    sem_wait(&plock);
    for (int row = 0; row < 3; row++) {
        printf("| ");
        for (int col = 0; col < 3; col++) {
            printf("%3d ", matrix[row][col]);
        }
        printf("|\n");
    }
    sem_post(&plock);
}

void zero_matrix(int target[3][3]) {
    for (int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            target[i][j] = 0;
        }
    }
}

void copy_data(const BMP_Image *in, channel_t channel, int target[3][3], int i, int j) {

#ifdef OLD_METHOD
    for(int row = 0; row < 3; row++) {
        if (i+row >= in->norm_height) {
            target[row][0] = 0;
            target[row][1] = 0;
            target[row][2] = 0;
            break;
        }
        for (int col = 0; col < 3; col++) {
            if (j + col >= in->norm_height) {
                target[row][col] = 0;
                continue;
            }
            Pixel curpixel = in->pixels[i + row][j + col];
            switch (channel) {
                case BLUE:
                target[row][col] = curpixel.blue;
                break;
                case RED:
                target[row][col] = curpixel.red;
                break;
                case GREEN:
                target[row][col] = curpixel.green;
                break;
                default:
                target[row][col] = curpixel.alpha;
            }
            
        }
    }
#else
    zero_matrix(target);
    // There are offsets
    int row = i == 0 ? 0 : -1;
    const int max_row = i < (in->norm_height - 1) ? 2 : 1;
    
    const int max_col = j < (abs(in->header.width_px) - 1) ? 2 : 1;
    const int init_col = j == 0 ? 0 : -1;
    
    for (; row < max_row; row++) {
        for (int col = init_col; col < max_col; col++) {
            switch(channel){ 
                case BLUE:
                target[row + 1][col + 1] = in->pixels[i + row][j + col].blue;
                break;
                case GREEN:
                target[row + 1][col + 1] = in->pixels[i + row][j + col].green;
                break;
                case RED:
                target[row + 1][col + 1] = in->pixels[i + row][j + col].red;
                break;
                default:
                break;
            }
            
        }
    }

#endif
}

void commit(BMP_Image *target, channel_t channel, int origin[3][3], int i, int j, uint8_t offset_r, uint8_t offset_c) {
    switch(channel) {
        case BLUE:
        target->pixels[i][j].blue = origin[1][1];
        break;
        case GREEN:
        target->pixels[i][j].green = origin[1][1];
        break;
        case RED:
        target->pixels[i][j].red = origin[1][1];
        break;
    }
}

void apply_conv(const int input[3][3], int output[3][3], int matrix[3][3], int factor) {

#ifdef OLD_METHOD
    for(int row = 0; row < 3; row++) {
        for(int col = 0; col < 3; col++) {
            int acc = 0;
            int counter = 0;
            // For some stupid reason this stop working
            for(int ofr = max(row-1, 0); ofr <= min(2, row + 1); ofr++) 
                for(int ofc = max(col - 1, 0); ofc <= min(2, col + 1); ofc++) {
                    acc += input[ofr][ofc] * matrix[row][col];
                    counter++;
                }
            output[row][col] = acc/(factor == 0 ? counter : factor);
        }
    }
#else
    int acc = 0;
    for(int row = 0; row < 3; row++){
        for (int col = 0; col < 3; col++) {
            acc += input[row][col] * matrix[row][col];
        }
    }
    output[1][1] = acc/factor;
#endif
}

void apply_simple_conv(const int input[3][3], int output[3][3], int factor) {
    int matrix[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1},
    };
    return apply_conv(input, output, matrix, factor);
}

void apply(BMP_Image * imageIn, BMP_Image * imageOut)
{
    sem_init(&serialsem, 0, 1);
#ifdef THREADED_APPLY_PRETEST
    int subpxls[3][3];
    int conv[3][3] = {{0}, {0},{0}};
    print_matrix(subpxls);
    printf("After copying data: \n");
    copy_data(imageIn, RED, subpxls, 1, 1);
    print_matrix(subpxls);
    apply_simple_conv(subpxls, conv, 0);
    printf("After applying convolution: \n");
    print_matrix(conv);
#endif
    // Ignore 1 for the main thread, if not enough it should be better to auto determine,
    // but 4 will be used as fallback
    const size_t total_threads = max(getCPUCount() - 1, 4);
#ifndef DEBUG_ONLY_HALF
#define EXPECTED_THREADS total_threads
#else
    printf("[WARN] Warning, using the configuration with the half of threads than expected\n");
#define EXPECTED_THREADS (total_threads/2)
#endif
    pthread_t* threads = malloc(sizeof(pthread_t) * total_threads);
    // pthread_attr_t thread_attrs[total_threads];
    int left_image_rows = imageIn->norm_height;

    const int EXPECTED_ROW_PER_THREAD = left_image_rows / total_threads;

    const double EXPECTED_FACTOR = (double)random() / RAND_MAX;
    printf("[INFO] Using the factor as %lf\n", EXPECTED_FACTOR);

    int rowstoscan = 0;

    for (int i = 0; i < EXPECTED_THREADS; i++) {
        worker_arg_t *thread_args = malloc(sizeof(worker_arg_t));
        thread_args->src = imageIn;
        thread_args->dest = imageOut;
        thread_args->init_col = 0;
        thread_args->row_count = (i == (total_threads - 1)) ? left_image_rows : EXPECTED_ROW_PER_THREAD;
        thread_args->init_row = i * EXPECTED_ROW_PER_THREAD;
        thread_args->factor = EXPECTED_FACTOR;
        left_image_rows -= EXPECTED_ROW_PER_THREAD;
        rowstoscan += thread_args->row_count;
        pthread_create(&threads[i], NULL, &filterThreadWorker, thread_args);
    }
#ifdef DEBUG_VERBOSE
    printf("[INFO]The rows to scan was set as: %d\n", rowstoscan);
#endif

    for (int i = 0; i < EXPECTED_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
    printf("The workers have ended\n");
    
}

void applyParallel(BMP_Image * imageIn, BMP_Image * imageOut, int boxFilter[3][3], int numThreads)
{}

/**
* Prevent that the calculated values overwrite another pixels that need more context
*/
void *filterThreadWorker(void * args) {
    worker_arg_t settings = *(worker_arg_t*) args;
    pthread_t id = acquire_id();
    printf("worker thread with id: %ld started \r\n", id);
    const int IMAGE_PWIDTH = settings.src->header.width_px;
    // BMP_Image *target = settings.dest;
    // Pixel **source = settings.src->pixels;
    const int START_ROW = settings.init_row;
    const int END_ROW = min(START_ROW + settings.row_count, settings.src->norm_height);
    // const double factor = settings.factor;


#ifdef SOLUTION_1
    int row = 0
#else
    int row = START_ROW;
#endif

    // Pixel box[3][3];
    for (; row < END_ROW; row++) {
#ifdef SOLUTION_1
        const int REAL_ROW = START_ROW + row;
        const int start_y = REAL_ROW == 0 ? 0 : -1;
        const int max_y = REAL_ROW == IMAGE_PWIDTH ? 0 : 1;
        const int diff_y = max_y - start_y + 1;

#endif
        for (int col = 0; col < IMAGE_PWIDTH; col++) {
#ifdef SOLUTION_1
            const int start_x = -1 * (col > 0);
            const int max_x = col < (IMAGE_PWIDTH - 1);
            const int diff_x = max_x - start_x + 1;
            Pixel pcount = { .blue=0,  .green=0, .red=0, .alpha= 255 };
            int count = diff_y * diff_x;
            for (int i = start_y; i <= max_y; i++) {
                for(int j = start_x; j <= max_x; j++) {
                    // prevent the access to ram, and rather use the value
                    Pixel cpixel = source[REAL_ROW + i][col+j];
                    pcount.blue += (double)cpixel.blue * factor;
                    pcount.red += (double)cpixel.red * factor;
                    pcount.green += (double)cpixel.green * factor;
                }
            }
            // Pixel tpixel = ;
            target->pixels[REAL_ROW][col].red = pcount.red / count ;
            // tpixel.blue = ((double)pcount.blue / count) * noise_factor;
            target->pixels[REAL_ROW][col].green = pcount.green / count;
            target->pixels[REAL_ROW][col].blue = pcount.blue / count;
#else
            // if (col == 0 || col == IMAGE_PWIDTH - 1) continue;
            int original[3][3];
            int targt[3][3];
            // Start calculating values from the previous row if possible
            // int real_row = START_ROW + row;
            // int real_col = col;
            // int working_row = max(START_ROW + row - 1, 0);
            // int working_col = max(col - 1, 0);
            #define FACTOR 9
            copy_data(settings.src, RED, original, row, col);
            apply_simple_conv(original, targt, FACTOR);
            commit(settings.dest, RED, targt, row, col, 0, 0);
            // Now apply convolution to the green channel
            copy_data(settings.src, GREEN, original, row, col);
            apply_simple_conv(original, targt, FACTOR);
            commit(settings.dest, GREEN, targt, row, col, 0, 0);
            // Now apply convolution for the blue
            copy_data(settings.src, BLUE, original, row, col);
            apply_simple_conv(original, targt, FACTOR);
            commit(settings.dest, BLUE, targt, row, col, 0, 0);
            
#endif
        }
    }
    
    free(args);
    return NULL;
}
