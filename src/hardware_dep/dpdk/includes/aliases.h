#ifndef __ALIASES_H_
#define __ALIASES_H_

#include <inttypes.h>

typedef struct rte_mbuf packet;
typedef struct lcore_conf configuration;

typedef enum field_data_type_e  field_data_type_t;
typedef enum header_instance_e  header_instance_t;
typedef enum field_type_e       field_type_t;

/* The index of a header in the header array. */
typedef uint8_t header_idx;
typedef uint8_t field;
typedef uint16_t length;

//=============================================================================
// Primitive actions

typedef struct packet_descriptor_s packet_descriptor_t;
typedef struct header_descriptor_s header_descriptor_t;

typedef struct header_reference_s  header_reference_t;
typedef struct field_reference_s  field_reference_t;


#endif // __ALIASES_H_

