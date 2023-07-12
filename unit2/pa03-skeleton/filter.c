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
#define abs(a) max(a, -a)
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

void copy_data(const BMP_Image *in, channel_t channel, int target[3][3], int i, int j) {
    for(int row = 0; row < 3; row++) {
        if (i+row >= in->norm_height) {
            target[row][0] = -1;
            target[row][1] = -1;
            target[row][2] = -1;
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
}

void commit(BMP_Image *target, channel_t channel, int origin[3][3], int i, int j, uint8_t offset_r, uint8_t offset_c) {
    for(int row = offset_r; row < 3; row++) {
        if (i + row - offset_r >= target->norm_height) {
            tprintf("[WARN] a commit for out of bound reached.\n");
            break;
        }
        for(int col = offset_c; col < 3; col++) {
            if ((i + row - offset_r) < 0) {
                tprintf("ERR: The calculus for the real pixel was wrong: { i: %d, row: %d, offset_r: %u }\n", i, row, offset_r);
            }
            if ((i + row - offset_r) > target->header.width_px) {
                tprintf("\n");
            }
            if ((j + col - offset_c) < 0) {
                tprintf("ERR: The calculus for the real pixel was wrong: { j: %d, col: %d, offset_c: %u }\n", j, col, offset_c);
            }
            switch (channel) {
                case BLUE:
                target->pixels[i + row - offset_r][j+col - offset_c].blue = origin[row][col];
                break;
                case GREEN:
                target->pixels[i + row - offset_r][j+col - offset_c].green = origin[row][col];
                break;
                case RED:
                // tprintf("image_dim:: {%dx%d}, commiting on red channel for pixel (%d, %d)\n", target->header.width_px, target->norm_height, i + row - offset_r, j+col - offset_c);
                target->pixels[i + row - offset_r][j+col - offset_c].red = origin[row][col];
                break;
                default:
                break;
            }
        }
    }
}

void apply_conv(const int input[3][3], int output[3][3], int matrix[3][3], int factor) {
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

void *filterThreadWorker(void * args) {
    worker_arg_t settings = *(worker_arg_t*) args;
    pthread_t id = acquire_id();
    printf("worker thread with id: %ld started \r\n", id);
#ifdef DEBUG_VERBOSE
    if (settings.skip) {
        printf("The thread %ld was set to skip filter\n", id);
        return NULL;
    }
#endif
    const int IMAGE_PWIDTH = settings.src->header.width_px;
    // const int IMAGE_PHEIGHT = settings.src->norm_height;
    BMP_Image *target = settings.dest;
    Pixel **source = settings.src->pixels;
    const int START_ROW = settings.init_row;
    const double factor = settings.factor;

#ifndef SOLUTION_1
#define next(var) (var++)
#else
#define next(var) var++
#endif

    // Pixel box[3][3];
    for (int row = 0; row < settings.row_count; next(row)) {
#ifdef SOLUTION_1
        const int REAL_ROW = START_ROW + row;
        const int start_y = REAL_ROW == 0 ? 0 : -1;
        const int max_y = REAL_ROW == IMAGE_PWIDTH ? 0 : 1;
        const int diff_y = max_y - start_y + 1;
#else
    // if (row == 0 || row == settings.row_count - 1) continue;
#endif
        for (int col = 0; col < IMAGE_PWIDTH-1; col++) {
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
            int real_row = START_ROW + row;
            int real_col = col;
            int working_row = max(START_ROW + row - 1, 0);
            int working_col = max(col - 1, 0);
            copy_data(settings.src, RED, original, working_row, max(col - 1, 0));
            apply_simple_conv(original, targt, 0);
            commit(settings.dest, RED, targt, real_row, real_col, diff(real_row, working_row), diff(real_col, working_col));
            // Now apply convolution to the green channel
            copy_data(settings.src, GREEN, original, working_row, max(col, 1));
            apply_simple_conv(original, targt, 0);
            commit(settings.dest, GREEN, targt, real_row, real_col, diff(real_row, working_row), diff(real_col, working_col));
            // Now apply convolution for the blue
            copy_data(settings.src, BLUE, original, working_row, max(col, 1));
            apply_simple_conv(original, targt, 0);
            commit(settings.dest, BLUE, targt, real_row, real_col, diff(real_row, working_row), diff(real_col, working_col));
            
#endif
        }
    }
    
    free(args);
    return NULL;
}
