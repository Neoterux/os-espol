/**
 * This code is deprecated for this work, would be worked better after.
 */
#ifndef COMMN_H
#define COMMN_H
#include <stdlib.h>
/**
 * The queue struct
 */
struct queue;

typedef struct queue queue_t;
/**************************************
 * This function will start the message queue between the child process
 * and the main thread.
 * will return the pointer that contains the queue value
 *
 * @param len The length of the queue
 *************************************/
queue_t* prepare_queue(const size_t len);

/*********************************************
 * Remove an item from the queue.
 *
 * @param queue the target queue
 * @return 1 if the pop was successfull, other value otherwise
 *********************************************/
int queue_pop(queue_t * queue, void *dest);

/********************************************
 * Check if the current queue is empty
 *
 * @param queue The queue to check
 * @return true if the queue is empty
 ********************************************/
int queue_is_empty(const queue_t queue);

/***********************************************
 * This method will store a value into the queue, the implementation for this
 * might be different dependending on the implementation and the values must
 *
 ***********************************************/
int queue_push(queue_t * queue, void* value, size_t vsize);

/*************************************************
 * This function will shutdown all the mechanisms
 * behind this queue, the implementation of this
 * depends on the backend behind this communication.
 *
 * @param queue The queue to shutdown
 * @return 0 if there was not problem, other value otherwise
 *************************************************/
int queue_shutdown(queue_t *queue);
#endif
