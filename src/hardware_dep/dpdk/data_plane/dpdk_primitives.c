#include "backend.h"
#include "dataplane.h"

#include <string.h> // memcpy

#include <rte_mempool.h>
#include <rte_mbuf.h>

#include <arpa/inet.h>

void
add_header(packet_descriptor_t* p, header_reference_t h)
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

void
remove_header(packet_descriptor_t* p, header_reference_t h)
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

uint32_t
extract_intvalue(packet_descriptor_t* p, field_reference_t f)
{
    if(f.bytewidth == 1)
        return (*(const uint8_t *)(FIELD_BYTE_ADDR(p, f)) & f.mask) >> f.bitoffset;
    else {
        uint32_t val;
        if(f.bytewidth == 2) {
            val = (*(const uint16_t *)(FIELD_BYTE_ADDR(p, f)) & f.mask) >> f.bitoffset;
            return f.meta ? val : ntohs(val);
        } else {
            if(f.bytewidth <= 4)
                val = (*(const uint32_t *)(FIELD_BYTE_ADDR(p, f)) & f.mask) >> f.bitoffset;
            else
                val = *(const uint32_t *)(FIELD_BYTE_ADDR(p, f) + f.bytewidth - sizeof(uint32_t));
            return f.meta ? val : ntohl(val);
        }
    }
}

uint8_t*
extract_bytebuf(packet_descriptor_t* p, field_reference_t f)
{
    if(f.bitwidth % 8 == 0 && f.bitoffset == 0)
        return FIELD_BYTE_ADDR(p, f);
    else
        //TODO
        return NULL;
}


/*
static void
add_int_values(uint8_t *dst_ptr, const uint8_t *src_ptr1, const int src_len1, const uint8_t *src_ptr2, const int src_len2)
{
    *(uint32_t *) dst_ptr = *(const uint32_t *)(src_ptr1 + src_len1 - sizeof(const uint32_t)) + *(const uint32_t *)(src_ptr2 + src_len2 - sizeof(const uint32_t));
}
*/
static void
add_to_int_field(packet_descriptor_t* p, field_reference_t f, uint32_t src_val)
{
    uint32_t res = extract_intvalue(p, f) + src_val; // TODO refine this operation
    modify_field_to_const32(p, f, res);
}

void
add_const_to_field(packet_descriptor_t* p, field_reference_t f, uint8_t *src, int srclen) {
    if(f.bytewidth <= 4) {
        //assert(field_instance_byte_width[f] == srclen); //TODO
        uint32_t src_val = 0;
        memcpy(&src_val, src, srclen);
        add_to_int_field(p, f, src_val);
    }
    //else
    //    assert("TODO");
}

void
modify_field_to_const32(packet_descriptor_t* p, field_reference_t f, uint32_t val)
{
    //assert(f.bytewidth <= 4);
    uint32_t nval = f.meta ? val : htonl(val);
    uint32_t mask = f.mask;
    uint32_t newint = (*(FIELD_BYTE_ADDR(p, f))&~mask)|(nval&(mask>>f.bitoffset))<<f.bitoffset;
    memcpy(FIELD_BYTE_ADDR(p, f), &newint, f.bytewidth);
}

void
modify_field_to_const(packet_descriptor_t* p, field_reference_t f, uint8_t *src, int srclen)
{
    if(f.bytewidth <= 4) {
        uint32_t src_val = 0;
        memcpy(&src_val, src, srclen);
        modify_field_to_const32(p, f, src_val);
    }
    else if(f.bytewidth == srclen)
        memcpy(FIELD_BYTE_ADDR(p, f), src, srclen);
    //else
    //    TODO
}

void
modify_field_to_field(packet_descriptor_t* p, field_reference_t dstf, field_reference_t srcf)
{
    if(dstf.bytewidth == srcf.bytewidth)
        modify_field_to_const(p, dstf, FIELD_BYTE_ADDR(p, srcf), srcf.bytewidth);
    //else
    //    assert("TODO");
}

void
add_fields(packet_descriptor_t* p, field_reference_t dstf, field_reference_t srcf1, field_reference_t srcf2) {
    if(dstf.bytewidth <= 4) {
        uint32_t val = extract_intvalue(p, srcf1) + extract_intvalue(p, srcf2); // TODO refine this add operation?
        modify_field_to_const32(p, dstf, val);
    }
    //else
    //    assert("TODO");
}

void
add_field_to_field(packet_descriptor_t* p, field_reference_t dstf, field_reference_t srcf)
{
    add_fields(p, dstf, dstf, srcf);
}

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

void copy_header(packet* p1, packet* p2, header_idx h1, header_idx h2, length l)
{
    // TODO
}

void drop(packet* p)
{
    // TODO
}
