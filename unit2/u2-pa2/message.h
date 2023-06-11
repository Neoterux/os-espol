#include <stddef.h>
#ifndef MESSAGE_H
#define MESSAGE_H

/**
 * @brief Initialize the communications betwee two or
 * more proccess.
 * 
 * @return int 0 if it was successful, -1 if not.
 */
int init_comms(const size_t comblksz);

/**
 * Send the given message to the another proccess.
 * 
 */
void send(const void *message);

int receive(void *message);

int shutdown_comms();
#endif
