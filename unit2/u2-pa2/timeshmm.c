#include "comm.h"

/*
 * Queue content
 */
struct queue {
    unsigned int queue_mlen;
    void** content;
    unsigned int fill_size;
    unsigned int _qpos;
};
