#include "message.h"
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

struct shmem_prop {
    /**
     * @brief The file descriptor of the shared memory
     * 
     */
    int shfd;
    /**
     * @brief The pointer of the shared memory object
     * 
     */
    void *shptr;
    /**
     * @brief Shared memory block size
     * 
     */
    size_t blksz;
};

struct shmem_prop shmem;
const char *SHMEM_NAME = "T_SHMEMM";

int init_comms(const size_t comblksz) {
    shmem.shfd = shm_open(SHMEM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shmem.shfd, comblksz);
    shmem.shptr = mmap(0, comblksz, PROT_WRITE, MAP_SHARED, shmem.shfd, 0);
    shmem.blksz = comblksz;
    return 0;
}

void send(const void *message) {
    write(shmem.shfd, message, shmem.blksz);
}

int receive(void *message) {
    memcpy(message, shmem.shptr, shmem.blksz);
    return 0;
}

int shutdown_comms() {
    return shm_unlink(SHMEM_NAME);
}
