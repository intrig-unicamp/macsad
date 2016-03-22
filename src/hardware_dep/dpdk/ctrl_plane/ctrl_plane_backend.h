#ifndef __BACKEND_H__
#define __BACKEND_H__ 1

#include <stdint.h>
#include "handlers.h"

typedef void* backend;
typedef void* digest;


backend create_backend(int num_of_threads, int queue_size, char* controller_name, int controller_port, p4_msg_callback cb);
void destroy_backend(backend bg);
int send_digest(backend bg, digest d, uint32_t receiver_id);
void launch_backend(backend bg);
void stop_backend(backend bg);

digest create_digest(backend bg, char* name);
digest add_digest_field(digest d, void* value, uint32_t length);


#endif
