
#include "backend.h"
#include "dataplane.h"

#if 0

// TODO push N elements
void
push(packet_descriptor_t* p, header_stack_t h)
{
    int next = 0;
    while(next < header_stack_size[h] && p->headers[header_stack_elements[h][next]].pointer != NULL) next++;
    debug("pushing next %d\n", next);
    int i;
    for(i = 0; i < next; i++)
        p->headers[header_stack_elements[h][i+1]].pointer = p->headers[header_stack_elements[h][i]].pointer;
    p->headers[header_stack_elements[h][0]].pointer =
        rte_pktmbuf_prepend(p->wrapper, p->headers[header_stack_elements[h][0]].length);
}

// TODO pop N elements
void
pop(packet_descriptor_t* p, header_stack_t h)
{
    int last = 0;
    while(last < header_stack_size[h] && p->headers[header_stack_elements[h][last]].pointer != NULL) last++;
    if(last > 0) {
        last--;
        debug("popping last %d\n", last);
        int i;
        for(i = 0; i < last; i++)
            p->headers[header_stack_elements[h][i]].pointer = p->headers[header_stack_elements[h][i+1]].pointer;
        // TODO: free up the corresponding part of the mbuf (rte_pktmbuf_adj is not appropriate here)
        p->headers[header_stack_elements[h][last]].pointer = NULL;
    }
    else debug("popping from empty header stack...\n");
}

void
add_header(packet_descriptor_t* p, header_reference_t h)
{
    if(p->headers[h.header_instance].pointer == NULL)
        p->headers[h.header_instance].pointer = rte_pktmbuf_prepend(p->wrapper, h.bytewidth);
    else
        debug("Cannot add a header instance already present in the packet\n");
}



void
remove_header(packet_descriptor_t* p, header_reference_t h)
{
    if(p->headers[h.header_instance].pointer != NULL) {
        // TODO: free up the corresponding part of the mbuf
        p->headers[h.header_instance].pointer = NULL;
    } else {
        debug("Cannot remove a header instance not present in the packet\n");
    }
}
#endif

void setValid(packet_descriptor_t* p, header_instance_t hdr_prefix)
{
    debug("calling setValid function\n");
    if(p->headers[hdr_prefix].pointer == NULL) {
        uint16_t len = header_instance_byte_width[hdr_prefix];
        uint32_t new = odp_packet_headroom (*((odp_packet_t *)(p->wrapper)));
        char* address = odp_packet_push_head(*((odp_packet_t *)(p->wrapper)), len);  // TODO if not to the front?
        //printf("################\n");
        //printf("headroom=%d\n",new);
        //printf("len=%d\n",len);
        //printf("address=%p\n",address);
        //printf("p->pointer=%p\n",p->pointer);

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

void setInvalid(packet_descriptor_t* p, header_instance_t hdr_prefix)
{
    debug("calling setInvalid function \n");
    //printf("p->headers[hdr_prefix].pointer = %p",p->headers[hdr_prefix].pointer);
    if(p->headers[hdr_prefix].pointer == NULL) {
        printf("Cannot remove a header instance not present in the packet\n");
    }
    else {
        //printf("\nRemoving header");
        uint16_t len = header_instance_byte_width[hdr_prefix];
        char* address =  odp_packet_pull_head(*((odp_packet_t *)(p->wrapper)), len);       // if not from the front?
        //printf("\nlen = %d", len);
        //printf("\np->wrapper = %d",p->wrapper);
      p->headers[hdr_prefix] =
           (header_descriptor_t) {
                .type = hdr_prefix,
                .pointer = address,
                .length = len
            };

}
}

void
generate_digest(ctrl_plane_backend bg, char* name, int receiver, struct type_field_list* digest_field_list)
{
    ctrl_plane_digest d = create_digest(bg, name);
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
