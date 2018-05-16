#include "backend.h"
#include "dataplane.h"

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
		debug("Cannot add a header instance already present in the packet\n");
	}
}

void remove_header(packet_descriptor_t* p, header_instance_t hdr_prefix)
{
	debug("calling remove_header \n");
	if(p->headers[hdr_prefix].pointer == NULL) {
		printf("Cannot remove a header instance not present in the packet\n");
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
		printf("copy_header failed with invalid hdr instance\n");
	}
	else {
		dlen = header_instance_byte_width[dhdr_prefix];
		slen = header_instance_byte_width[shdr_prefix];
		if (dlen!=slen) {
			debug("copy_header failed with mismatch hdr lenght\n");
		}
		//int result = odp_packet_copy_data(*((odp_packet_t *)(p->wrapper)), 0, 1, len);
		info("\e[35m copying %d bytes from hdr_instance %d to %d \n", slen, shdr_prefix, dhdr_prefix);
		memcpy(p->headers[dhdr_prefix].pointer, p->headers[shdr_prefix].pointer, slen);
	}
}

uint16_t modify_field_with_hash_based_offset(int result, struct type_field_list* field){
    uint16_t hash = 0;
	int i;
	for(i = 0; i < field->fields_quantity; i++)
		hash = hash + calculate_csum16(field->field_offsets[i], field->field_widths[i]);
    uint16_t size = 2;
	printf("\n\n\n\n O hash é: %d \n\n\n\n",hash)
    info("applying hash \n");
    result = hash % size;
	printf("\n\n\n\n\n###### O resultado é: %d ###### \n\n\n\n\n", result);
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
