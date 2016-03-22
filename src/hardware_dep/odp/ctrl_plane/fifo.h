#ifndef __FIFO_H__
#define __FIFO_H__

#include <pthread.h>

#define P4_BG_QUEUE_SIZE 1024

typedef struct fifo_st {
        void* queue[P4_BG_QUEUE_SIZE];
        int head;
        int size;
        int tail;
        pthread_mutex_t lock;
        pthread_cond_t not_empty;     /*non empty and empty condidtion variables*/
        pthread_cond_t empty;
} fifo_t;

fifo_t* fifo_init( fifo_t* queue );
void fifo_destroy( fifo_t* queue );
fifo_t* fifo_add_msg( fifo_t* queue, void* element);
void* fifo_remove_msg( fifo_t* queue );
int fifo_size( fifo_t* queue );
void fifo_wait( fifo_t* queue );

#endif
