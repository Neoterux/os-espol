#include "message.h"
#include <stddef.h>
#include <unistd.h>

struct commstg_t {
    size_t blksz;
    int ctrnpipe[2];
};

struct commstg_t comm;

int init_comms(const size_t comblksz) {
    int result = pipe(comm.ctrnpipe);
    comm.blksz = comblksz;
    return result;
}

void send(const void *message) {
    write(comm.ctrnpipe[1], message, comm.blksz);
}

int receive(void *message) {
    return read(comm.ctrnpipe[0], message, comm.blksz);
}

int shutdown_comms() {
    int outres = close(comm.ctrnpipe[0]);
    int inres = close(comm.ctrnpipe[1]);
    return outres < 0 ? outres : (inres < 0 ? inres : 0);
}
