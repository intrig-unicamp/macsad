#ifndef DATAPLANE_H
#define DATAPLANE_H

#include <inttypes.h>
#include "aliases.h"
#include "parser.h"

#include <odp_api.h>
//TODO
//#include <rte_atomic.h>

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
    uint8_t min_size;
    uint8_t max_size;
    void* default_val;
    void* table;
    int socketid;
} lookup_table_t;

typedef struct counter_s {
	char* name;
	uint8_t type;
	uint8_t min_width;
	uint8_t size;
//TODO
//	rte_atomic32_t *cnt; // volatile ints
	int socketid;
} counter_t;

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
//void handle_packet(packet_descriptor_t* packet);

#define FIELD_BYTE_ADDR(p, f) (((uint8_t*)(p)->headers[f.header].pointer)+f.byteoffset)

#define MODIFY_BYTEBUF_BYTEBUF(pd, dstfield, src, srclen) { \
    memcpy(FIELD_BYTE_ADDR(pd, field_desc(dstfield)), src, srclen); \
}

// supposing `uint32_t value32' is in the scope
#define MODIFY_INT32_BYTEBUF(pd, dstfield, src, srclen) { \
    value32 = 0; \
    memcpy(&value32, src, srclen); \
    MODIFY_INT32_INT32(pd, dstfield, value32); \
}

// supposing `uint32_t res32' is in the scope
#define MODIFY_INT32_INT32(pd, dstfield, value32) { \
    res32 = (*FIELD_BYTE_ADDR(pd, field_desc(dstfield)) & ~field_desc(dstfield).mask) | (value32 & (field_desc(dstfield).mask >> field_desc(dstfield).bitoffset)) << field_desc(dstfield).bitoffset; \
    memcpy(FIELD_BYTE_ADDR(pd, field_desc(dstfield)), &res32, field_desc(dstfield).bytewidth); /* the compiler optimises this and cuts off the shifting/masking stuff whenever the bitoffset is 0 */ \
}

#define NTOH16(val, field) (field_desc(field).meta ? val : rte_be_to_cpu_16(val))
#define NTOH32(val, field) (field_desc(field).meta ? val : rte_be_to_cpu_32(val))

#define EXTRACT_INT32(pd, field, dst) { \
    if(field_desc(field).bitwidth <= 8) \
        dst = (uint32_t)((*(uint8_t *)FIELD_BYTE_ADDR(pd, field_desc(field)) & (uint8_t)field_desc(field).mask) >> field_desc(field).bitoffset); \
    else if(field_desc(field).bitwidth <= 16) \
        dst = (uint32_t)NTOH16(((*(uint16_t *)FIELD_BYTE_ADDR(pd, field_desc(field)) & (uint16_t)field_desc(field).mask) >> field_desc(field).bitoffset), field); \
    else \
        dst = (uint32_t)NTOH32(((*(uint32_t *)(FIELD_BYTE_ADDR(pd, field_desc(field))) & (uint32_t)field_desc(field).mask) >> field_desc(field).bitoffset), field); \
}


#define EXTRACT_32BITS(pd, field, dst) { \
    if(field_desc(field).bitwidth <= 8) \
        dst = (uint32_t)((*(uint8_t *)FIELD_BYTE_ADDR(pd, field_desc(field)) & (uint8_t)field_desc(field).mask) >> field_desc(field).bitoffset); \
    else if(field_desc(field).bitwidth <= 16) \
        dst = (uint32_t)((*(uint16_t *)FIELD_BYTE_ADDR(pd, field_desc(field)) & (uint16_t)field_desc(field).mask) >> field_desc(field).bitoffset); \
    else \
        dst = (uint32_t)((*(uint32_t *)(FIELD_BYTE_ADDR(pd, field_desc(field))) & (uint32_t)field_desc(field).mask) >> field_desc(field).bitoffset); \
}

#define EXTRACT_BYTEBUF(pd, field, dst) { \
    memcpy(dst, FIELD_BYTE_ADDR(pd, field_desc(field)), field_desc(field).bytewidth); \
}

#endif
