/**
 * threadpool.h
 *
 * This file declares the functionality associated with
 * your implementation of a threadpool.
 */


#define MAXT_IN_POOL 200







typedef void *threadpool;







typedef void (*dispatch_fn)(void *);

/**
 * create_threadpool creates a fixed-sized thread
 * pool.  If the function succeeds, it returns a (non-NULL)
 * "threadpool", else it returns NULL.
 */
threadpool create_threadpool(int num_threads_in_pool);


/**
 * dispatch sends a thread off to do some work.  If
 * all threads in the pool are busy, dispatch will
 * block until a thread becomes free and is dispatched.
 * 
 * Once a thread is dispatched, this function returns
 * immediately.
 * 
 * The dispatched thread calls into the function
 * "dispatch_to_here" with argument "arg".
 */
void dispatch(threadpool from_me, dispatch_fn dispatch_to_here, void *arg);

/**
 * destroy_threadpool kills the threadpool, causing
 * all threads in it to commit suicide, and then
 * frees all the memory associated with the threadpool.
 */
void destroy_threadpool(threadpool destroyme);

