#include "backend.h"
#include "dataplane.h"

#if 0
void add_header(packet_descriptor_t* p, header_reference_t h)
{
    if(p->headers[h.header_instance].pointer == NULL) {
        uint16_t len = h.bytewidth;
        char* address = rte_pktmbuf_prepend(p->pointer, len); // if not to the front?
        p->headers[h.header_instance] =
            (header_descriptor_t) {
                .type = h.header_instance,
                .pointer = address,
                .length = h.bytewidth // max_width?
            };
    } else {
        printf("Cannot add a header instance already present in the packet\n");
    }
}

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
