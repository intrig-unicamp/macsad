#ifndef DATAPLANE_H
#define DATAPLANE_H

#include <inttypes.h>
#include "aliases.h"
#include "parser.h"
#include "data_plane_data.h"

#define LOOKUP_EXACT   0
#define LOOKUP_LPM     1
#define LOOKUP_TERNARY 2

typedef struct lookup_table_s {
    char* name;
	uint8_t type;
    uint8_t key_size;
    uint8_t val_size;
    uint8_t min_size;
    uint8_t max_size;
	void* default_val;
	void* table;
} lookup_table_t;


// TODO remove these, or change packets.h
typedef enum field_data_type_e      field_data_type_t;
typedef enum header_instance_e      header_instance_t;
typedef enum field_instance_e       field_instance_t;


typedef struct field_reference_s {
    header_instance_t header;
    int meta;
    int bitwidth;
    int bytewidth;
    int bitoffset;
    int byteoffset;
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
                 .byteoffset = field_instance_byte_offset_hdr[f] \
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
} packet_descriptor_t;

//=============================================================================
// Callbacks

extern lookup_table_t table_config[];

void handle_packet(packet_descriptor_t* packet, lookup_table_t** tables);

#endif
