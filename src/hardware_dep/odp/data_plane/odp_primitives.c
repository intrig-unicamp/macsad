
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
