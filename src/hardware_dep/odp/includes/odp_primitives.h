#ifndef ODP_PRIMITIVES_H
#define ODP_PRIMITIVES_H

//#include <rte_byteorder.h>
#include <odp_api.h>

#define FIELD_BYTE_ADDR(p, f) (((uint8_t*)(p)->headers[f.header].pointer)+f.byteoffset)

#define MODIFY_BYTEBUF_BYTEBUF(pd, dstfield, src, srclen) { \
    memcpy(FIELD_BYTE_ADDR(pd, field_desc(dstfield)), src, srclen); \
}

// assuming `uint32_t value32' is in the scope
#define MODIFY_INT32_BYTEBUF(pd, dstfield, src, srclen) { \
    value32 = 0; \
    memcpy(&value32, src, srclen); \
    MODIFY_INT32_INT32(pd, dstfield, value32); \
}

// assuming `uint32_t res32' is in the scope
#define MODIFY_INT32_INT32(pd, dstfield, value32) { \
    res32 = (*FIELD_BYTE_ADDR(pd, field_desc(dstfield)) & ~field_desc(dstfield).mask) | (value32 & (field_desc(dstfield).mask >> field_desc(dstfield).bitoffset)) << field_desc(dstfield).bitoffset; \
    memcpy(FIELD_BYTE_ADDR(pd, field_desc(dstfield)), &res32, field_desc(dstfield).bytewidth); /* the compiler optimises this and cuts off the shifting/masking stuff whenever the bitoffset is 0 */ \
}

#define NTOH16(val, field) (field_desc(field).meta ? val : odp_be_to_cpu_16(val))
#define NTOH32(val, field) (field_desc(field).meta ? val : odp_be_to_cpu_32(val))

#define EXTRACT_INT32(pd, field, dst) { \
    if(field_desc(field).bitwidth <= 8) \
        dst = (uint32_t)((*(uint8_t *)FIELD_BYTE_ADDR(pd, field_desc(field)) & (uint8_t)field_desc(field).mask) >> field_desc(field).bitoffset); \
    else if(field_desc(field).bitwidth <= 16) \
        dst = (uint32_t)NTOH16(((*(uint16_t *)FIELD_BYTE_ADDR(pd, field_desc(field)) & (uint16_t)field_desc(field).mask) >> field_desc(field).bitoffset), field); \
    else \
        dst = (uint32_t)NTOH32(((*(uint32_t *)(FIELD_BYTE_ADDR(pd, field_desc(field))) & (uint32_t)field_desc(field).mask) >> field_desc(field).bitoffset), field); \
}

#define EXTRACT_32BITS(pd, field, dst) { \
    if(field_desc(field).bitwidth <= 8) \
        dst = (uint32_t)((*(uint8_t *)FIELD_BYTE_ADDR(pd, field_desc(field)) & (uint8_t)field_desc(field).mask) >> field_desc(field).bitoffset); \
    else if(field_desc(field).bitwidth <= 16) \
        dst = (uint32_t)((*(uint16_t *)FIELD_BYTE_ADDR(pd, field_desc(field)) & (uint16_t)field_desc(field).mask) >> field_desc(field).bitoffset); \
    else \
        dst = (uint32_t)((*(uint32_t *)(FIELD_BYTE_ADDR(pd, field_desc(field))) & (uint32_t)field_desc(field).mask) >> field_desc(field).bitoffset); \
}

#define EXTRACT_BYTEBUF(pd, field, dst) { \
    memcpy(dst, FIELD_BYTE_ADDR(pd, field_desc(field)), field_desc(field).bytewidth); \
}

#endif // ODP_PRIMITIVES_H
