#ifndef __BACKEND_H_
#define __BACKEND_H_

#include "aliases.h"
#include "ctrl_plane_backend.h"
#include "data_plane_data.h"
#include "dataplane.h"

//=============================================================================
// General

uint8_t       initialize   (int argc, char **argv);
int           launch       (void);

//=============================================================================
// Table mgmt

typedef struct lookup_table_s lookup_table_t;

void        table_create (lookup_table_t* t, int socketid, int replicaid);

void    table_setdefault (lookup_table_t* t,                              uint8_t* value);

void           exact_add (lookup_table_t* t, uint8_t* key,                uint8_t* value);
void             lpm_add (lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value);
void         ternary_add (lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value);

uint8_t*    exact_lookup (lookup_table_t* t, uint8_t* key);
uint8_t*      lpm_lookup (lookup_table_t* t, uint8_t* key);
uint8_t*  ternary_lookup (lookup_table_t* t, uint8_t* key);

//=============================================================================
// Primitive actions

typedef struct packet_descriptor_s packet_descriptor_t;
typedef struct header_descriptor_s header_descriptor_t;

typedef struct header_reference_s  header_reference_t;
typedef struct field_reference_s  field_reference_t;

//=============================================================================
// New generation

void add_header             (packet_descriptor_t* p, header_reference_t h);
void remove_header          (packet_descriptor_t* p, header_reference_t h);

// PRIMITIVE: MODIFY_FIELD
void modify_field_to_const  (packet_descriptor_t* p, field_reference_t f,    uint8_t *src, int srclen);
void modify_field_to_const32(packet_descriptor_t* p, field_reference_t f,    uint32_t val);
void modify_field_to_field  (packet_descriptor_t* p, field_reference_t dstf, field_reference_t srcf);

// PRIMITIVE: ADD_TO_FIELD
void add_const_to_field     (packet_descriptor_t* p, field_reference_t f,    uint8_t *src, int srclen);
void add_field_to_field     (packet_descriptor_t* p, field_reference_t dstf, field_reference_t srcf);

// PRIMITIVE: ADD
void add_fields             (packet_descriptor_t* p, field_reference_t dstf, field_reference_t srcf1, field_reference_t srcf2);
//void add_field_const        (packet_descriptor_t* p, field_reference_t dstf, field_reference_t srcf, uint8_t *src, int srclen);
//void add_consts             (packet_descriptor_t* p, field_reference_t dstf, uint8_t *src1, int src1len, uint8_t *src2, int src2len);

// PRIMITIVE: DROP
void drop (packet_descriptor_t* p);

// EXTRA
uint32_t extract_intvalue   (packet_descriptor_t* p, field_reference_t f);
uint8_t* extract_bytebuf    (packet_descriptor_t* p, field_reference_t f);


//=============================================================================
// Old generation

//void copy_header            (packet* p1, packet* p2, header_idx h1, header_idx h2, length l);

void modify_field           (packet* p, field f, length l, void* value, int modify_length);
void modify_field_with_mask (packet* p, field f, void* value, void* mask, int modify_length);
void modify_field_const_long(packet* p, field f, length l, long value, int modify_length);
void add_to_field           (packet* p, field* f, void* value);
void add                    (packet* p, field* f, void* value1, void* value2);
void set_field_to_hash_index(packet* p, field* f, field* flc, int base, int size);
void truncate_pkg           (packet* p, unsigned length);
void no_op                  ();
void push                   (packet* p, header_idx* hdr_array);
void push_n                 (packet* p, header_idx* hdr_array, unsigned count);
void pop                    (packet* p, header_idx* hdr_array);
void pop_n                  (packet* p, header_idx* hdr_array, unsigned count);
//count
//meter
void generate_digest        (backend bg, char* name, int receiver, struct type_field_list* digest_field_list);
//resubmit
//recirculate
//clone_ingress_pkt_to_ingress
//clone_egress_pkt_to_ingress
//clone_ingress_pkt_to_egress
//clone_egress_pkt_to_egress

#endif // __BACKEND_H_
