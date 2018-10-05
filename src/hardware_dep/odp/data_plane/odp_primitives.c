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

#include <string.h>
#include "backend.h"
#include "dataplane.h"

uint16_t calculate_csum16(void* buf, uint16_t length);

void add_header(packet_descriptor_t* p, header_instance_t hdr_prefix)
{
	debug("calling add_header \n");
	if(p->headers[hdr_prefix].pointer == NULL) {
		uint16_t len = header_instance_byte_width[hdr_prefix];
		char* address = odp_packet_push_head(*((odp_packet_t *)(p->wrapper)), len);
		p->headers[hdr_prefix] =
			(header_descriptor_t) {
				.type = hdr_prefix,
				.pointer = address,
				.length = len
			};
	} else {
		warn("Cannot add a header instance already present in the packet\n");
	}
}

void remove_header(packet_descriptor_t* p, header_instance_t hdr_prefix)
{
	debug("calling remove_header \n");
	if(p->headers[hdr_prefix].pointer == NULL) {
		warn("Cannot remove a header instance not present in the packet\n");
	}
	else {
		uint16_t len = header_instance_byte_width[hdr_prefix];
		char* address =  odp_packet_pull_head(*((odp_packet_t *)(p->wrapper)), len);
		p->headers[hdr_prefix] =
			(header_descriptor_t) {
				.type = hdr_prefix,
				.pointer = address,
				.length = len
			};

	}
}

void copy_header(packet_descriptor_t* p, header_instance_t dhdr_prefix, header_instance_t shdr_prefix)
{
	uint16_t dlen = 0, slen = 0;
	debug("calling copy_header \n");

	if((p->headers[dhdr_prefix].pointer == NULL) || (p->headers[dhdr_prefix].pointer == NULL)) {
		warn("copy_header failed with invalid hdr instance\n");
	}
	else {
		dlen = header_instance_byte_width[dhdr_prefix];
		slen = header_instance_byte_width[shdr_prefix];
		if (dlen!=slen) {
			warn("copy_header failed with mismatch hdr lenght\n");
		}
		//int result = odp_packet_copy_data(*((odp_packet_t *)(p->wrapper)), 0, 1, len);
		info("copying %d bytes from hdr_instance %d to %d \n", slen, shdr_prefix, dhdr_prefix);
		memcpy(p->headers[dhdr_prefix].pointer, p->headers[shdr_prefix].pointer, slen);
	}
}

uint16_t modify_field_with_hash_based_offset(int result, struct type_field_list* field, int size){
    uint16_t hash = 0;
	int i;
	for(i = 0; i < field->fields_quantity; i++)
		hash = hash + calculate_csum16(field->field_offsets[i], field->field_widths[i]);
    info("applying hash \n");
    result = hash % size;
    return result;
}

void generate_digest(backend bg, char* name, int receiver, struct type_field_list* digest_field_list)
{
	digest d = create_digest(bg, name);
	int i;
	for(i = 0; i < digest_field_list->fields_quantity; i++)
		d = add_digest_field(d, digest_field_list->field_offsets[i], digest_field_list->field_widths[i]);
	send_digest(bg, d, receiver);
}

void no_op()
{
}

void resubmit(packet_descriptor_t* p)
{
	// TODO
}


void drop(packet_descriptor_t* p)
{
	// TODO
}
