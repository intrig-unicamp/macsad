#include "backend.h"
#include "dataplane.h"

void add_header(packet_descriptor_t* p, header_instance_t hdr_prefix)
{
    debug("calling add_header \n");
    if(p->headers[hdr_prefix].pointer == NULL) {
        uint16_t len = header_instance_byte_width[hdr_prefix];
        uint32_t new = odp_packet_headroom (*((odp_packet_t *)(p->wrapper)));
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

void remove_header(packet_descriptor_t* p, header_reference_t hdr_prefix)
{
    debug("calling remove_header \n");
    if(p->headers[hdr_prefix].pointer == NULL) {
        printf("Cannot remove a header instance not present in the packet\n");
    }
    else {
        uint16_t len = header_instance_byte_width[hdr_prefix];
        char* address =  odp_packet_pull_head(*((odp_packet_t *)(p->wrapper)), len);           p->headers[hdr_prefix] =
           (header_descriptor_t) {
                .type = hdr_prefix,
                .pointer = address,
                .length = len
            };

    }
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
