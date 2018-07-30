// Copyright 2018 INTRIG/FEEC/UNICAMP (University of Campinas), Brazil
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

#ifndef __BACKEND_H_
#define __BACKEND_H_

#include <stdio.h>
#include "aliases.h"
#include "ctrl_plane_backend.h"
#include "data_plane_data.h"
#include "dataplane.h"

#ifdef NDEBUG
#define debug(args, ...)
#else
#define debug(args, ...) fprintf(stderr, "[DEBUG] \x1b[35m %s:%d " args "\n \x1b[0m", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef NINFO
#define info(args, ...)
#else
#define info(args, ...) fprintf(stderr, "[INFO] \x1b[34m %s:%d " args "\n \x1b[0m", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef NSIGG
#define sigg(arg ,args, ...)
#else
#define sigg(arg, args, ...) fprintf(stdout, arg "\x1b[32m " args "\x1b[0m \n", ##__VA_ARGS__)
#endif

#ifdef NERROR
#define error(args, ...)
#else
#define error(args, ...) fprintf(stderr, args "[ERROR] \x1b[31m %s:%d " args "\n \x1b[0m", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef NWARN
#define warn(args, ...)
#else
#define warn(args, ...) fprintf(stderr, args "[WARNING] \x1b[33m %s:%d " args "\n \x1b[0m", __FILE__, __LINE__, ##__VA_ARGS__)
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

void add_header      (packet_descriptor_t* p, header_instance_t h);
void remove_header   (packet_descriptor_t* p, header_instance_t h);
void copy_header     (packet_descriptor_t* p, header_instance_t dh, header_instance_t sh);
void drop            (packet_descriptor_t* p);
void generate_digest (backend bg, char* name, int receiver, struct type_field_list* digest_field_list);
void no_op ();

//=============================================================================
// Calculations
//uint16_t calculate_csum16(const void* buf, uint16_t length);
//uint32_t packet_length(packet_descriptor_t* pd);

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
