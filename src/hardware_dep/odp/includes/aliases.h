#ifndef __ALIASES_H_
#define __ALIASES_H_

#include <odp_api.h>

// the shared dataplane.h refers to it
typedef struct odp_packet_t packet;
typedef struct lcore_conf configuration;

//#include <spinlock.h>
typedef odp_spinlock_t lock;

#define DEFAULT_ACTION_INDEX -42

#endif // __ALIASES_H_

