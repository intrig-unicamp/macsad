#ifndef __BACKEND_H_
#define __BACKEND_H_

#include "aliases.h"
#include "ctrl_plane_backend.h"
#include "data_plane_data.h"
#include "dataplane.h"

#ifdef NDEBUG
#define debug(args, ...)
#else
#define debug(args, ...) fprintf(stderr, "[DEBUG] %s:%d " args "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef NINFO
#define info(args, ...)
#else
#define info(args, ...) fprintf(stderr, "[INFO] %s:%d " args "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef NSIGG
#define sigg(args, ...)
#else
#define sigg(args, ...) fprintf(stdout, args "\n", ##__VA_ARGS__)
#endif

//=============================================================================
// General

uint8_t       initialize   (int argc, char **argv);
int           launch       (void);

//=============================================================================
// Table mgmt

void table_create (lookup_table_t* t, int socketid, int replicaid);
void table_setdefault (lookup_table_t* t, uint8_t* value);

void exact_add (lookup_table_t* t, uint8_t* key, uint8_t* value);
void lpm_add (lookup_table_t* t, uint8_t* key, uint8_t depth, uint8_t* value);
void ternary_add (lookup_table_t* t, uint8_t* key, uint8_t* mask, uint8_t* value);

uint8_t* exact_lookup (lookup_table_t* t, uint8_t* key);
uint8_t* lpm_lookup (lookup_table_t* t, uint8_t* key);
uint8_t* ternary_lookup (lookup_table_t* t, uint8_t* key);

//=============================================================================
// Primitive actions

void add_header      (packet_descriptor_t* p, header_reference_t h);
void remove_header   (packet_descriptor_t* p, header_reference_t h);
void drop            (packet_descriptor_t* p);
void generate_digest (backend bg, char* name, int receiver, struct type_field_list* digest_field_list);
void no_op ();

//=============================================================================
// Calculations
//uint16_t calculate_csum16(const void* buf, uint16_t length);
//uint32_t packet_length(packet_descriptor_t* pd);

//copy_header
////void set_field_to_hash_index(packet* p, field* f, field* flc, int base, int size);/
////void truncate_pkg           (packet* p, unsigned length);
////void push                   (packet* p, header_idx* hdr_array);
////void push_n                 (packet* p, header_idx* hdr_array, unsigned count);
////void pop                    (packet* p, header_idx* hdr_array);
////void pop_n                  (packet* p, header_idx* hdr_array, unsigned count);
////count
////meter
////resubmit
////recirculate
////clone_ingress_pkt_to_ingress
////clone_egress_pkt_to_ingress
////clone_ingress_pkt_to_egress
////clone_egress_pkt_to_egress

#endif // __BACKEND_H_
