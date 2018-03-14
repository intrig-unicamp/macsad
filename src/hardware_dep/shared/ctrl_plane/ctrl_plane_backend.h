#ifndef __BACKEND_H__
#define __BACKEND_H__ 1

#include <stdint.h>
#include "handlers.h"

typedef void* ctrl_plane_backend;
typedef void* ctrl_plane_digest;


ctrl_plane_backend create_backend(int num_of_threads, int queue_size, char* controller_name, int controller_port, p4_msg_callback cb);
void destroy_backend(ctrl_plane_backend bg);
int send_digest(ctrl_plane_backend bg, ctrl_plane_digest d, uint32_t receiver_id);
void launch_backend(ctrl_plane_backend bg);
void stop_backend(ctrl_plane_backend bg);

ctrl_plane_digest create_digest(ctrl_plane_backend bg, char* name);
ctrl_plane_digest add_digest_field(ctrl_plane_digest d, void* value, uint32_t length);


#endif
