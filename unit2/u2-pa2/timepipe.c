#include "comm.h"
#include <stddef.h>

struct queue {
    unsigned int queue_mlen;
    void** content;
    unsigned int fill_size;
    unsigned int _qpos;
};


queue_t* prepare_queue(const size_t len) {
    return NULL;
}

int queue_pop(queue_t *queue, void *dest) {
    return 0;
}

int queue_is_empty(const queue_t queue) {
    return 0;
}

int queue_push(queue_t *queue, void* value, size_t vsize) {
    return 0;
}

int queue_shutdown(const queue_t *queue) {
    return 0;
}
