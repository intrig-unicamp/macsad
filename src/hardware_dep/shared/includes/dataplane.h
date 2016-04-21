#ifndef DATAPLANE_H
#define DATAPLANE_H

#include <inttypes.h>
#include "aliases.h"
#include "parser.h"
#include "vector.h"

#include <odp_api.h>

#define LOOKUP_EXACT   0
#define LOOKUP_LPM     1
#define LOOKUP_TERNARY 2

#define MACS_TABLE_NAME_LEN 64

struct type_field_list {
    uint8_t fields_quantity;
    uint8_t** field_offsets;
    uint8_t* field_widths;
};

typedef struct lookup_table_s {
    char* name;
    uint8_t type;
    uint8_t key_size;
    uint8_t val_size;
    int min_size;
    int max_size;
	int counter;
    void* default_val;
    void* table;
    int socketid;
} lookup_table_t;

typedef struct counter_s {
	char* name;
	uint8_t type;
	uint8_t min_width;
	int size;
	uint8_t saturating;
	vector_t *values; //rte_atomic32_t *cnt; // volatile ints
	int socketid;
} counter_t;

typedef struct field_reference_s {
    header_instance_t header;
    int meta;
    int bitwidth;
    int bytewidth;
    int bitoffset;
    int byteoffset;
    uint32_t mask;
} field_reference_t;

typedef struct header_reference_s {
    header_instance_t header_instance;
    int bytewidth;
} header_reference_t;

#define field_desc(f) (field_reference_t) \
               { \
                 .header     = field_instance_header[f], \
                 .meta       = header_instance_is_metadata[field_instance_header[f]], \
                 .bitwidth   = field_instance_bit_width[f], \
                 .bytewidth  = (field_instance_bit_width[f]+7)/8, \
                 .bitoffset  = field_instance_bit_offset[f], \
                 .byteoffset = field_instance_byte_offset_hdr[f], \
                 .mask       = field_instance_mask[f] \
               }

#define header_desc(h) (header_reference_t) \
               { \
                 .header_instance = h, \
                 .bytewidth       = header_instance_byte_width[h], \
               }

typedef struct header_descriptor_s {
    header_instance_t   type;
    void *              pointer;
    uint32_t            length;
} header_descriptor_t;

typedef struct packet_descriptor_s {
    void *              pointer;
    header_descriptor_t headers[HEADER_INSTANCE_COUNT+1];
    packet *		packet;
} packet_descriptor_t;


//=============================================================================
// Callbacks

extern lookup_table_t table_config[];
extern counter_t counter_config[];

void handle_packet(packet_descriptor_t* packet, lookup_table_t** tables);

#endif //DATAPLANE_H
