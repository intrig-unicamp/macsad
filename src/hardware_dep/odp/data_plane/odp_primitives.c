#include "backend.h"
#include "dataplane.h"

//void add_header(packet_descriptor_t* p, uint8_t hdr_prefix)
void add_header(packet_descriptor_t* p, header_instance_t hdr_prefix)
{
    debug("calling add_header \n");
    if(p->headers[hdr_prefix].pointer == NULL) {
        uint16_t len = header_instance_byte_width[hdr_prefix];
        char* address = odp_packet_push_head(p->pointer, len); // TODO if not to the front?
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

#if 0
void remove_header(packet_descriptor_t* p, header_reference_t h)
{
    header_descriptor_t hd = p->headers[h.header_instance];
    if(hd.pointer != NULL) {
        uint16_t len = (uint16_t)hd.length;
        rte_pktmbuf_adj(p->pointer, len);  // if not from the front?
        p->headers[h.header_instance].length = 0;
        p->headers[h.header_instance].pointer = NULL;
    } else {
        printf("Cannot remove a header instance not present in the packet\n");
    }
}
#endif

void
generate_digest(backend bg, char* name, int receiver, struct type_field_list* digest_field_list)
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
