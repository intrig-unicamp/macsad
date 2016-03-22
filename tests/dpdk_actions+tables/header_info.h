#ifndef __HEADER_INFO_H__
#define __HEADER_INFO_H__

#define HEADER_INSTANCE_COUNT 2+2
#define FIELD_INSTANCE_COUNT 15+9

enum header_instance_e {
    header_instance_NONE,
    /* virtual must go first */
    header_instance_ethernet,
    header_instance_ipv4,
    header_instance_standard_metadata,
    header_instance_COUNT
};

enum field_instance_e {
    field_instance_NONE,
    /* virtual must go first */
    field_instance_ethernet_dstAddr,
    field_instance_ethernet_srcAddr,
    field_instance_ethernet_etherType,
    field_instance_ipv4_version,
    field_instance_ipv4_ihl,
    field_instance_ipv4_diffserv,
    field_instance_ipv4_totalLen,
    field_instance_ipv4_identification,
    field_instance_ipv4_flags,
    field_instance_ipv4_fragOffset,
    field_instance_ipv4_ttl,
    field_instance_ipv4_protocol,
    field_instance_ipv4_hdrChecksum,
    field_instance_ipv4_srcAddr,
    field_instance_ipv4_dstAddr,

    field_instance_standard_metadata_ingress_port,
    field_instance_standard_metadata_packet_length,
    field_instance_standard_metadata_egress_spec,
    field_instance_standard_metadata_egress_port,
    field_instance_standard_metadata_egress_instance,
    field_instance_standard_metadata_instance_type,
    field_instance_standard_metadata_clone_spec,
    field_instance_standard_metadata__padding,
    field_instance_COUNT
};

typedef enum field_data_type_e      field_data_type_t;
typedef enum header_instance_e      header_instance_t;
typedef enum field_instance_e       field_instance_t;

static const int header_instance_byte_width[HEADER_INSTANCE_COUNT + 1] = {
  -1,  /* header_instance_NONE */
  14,  /* header_instance_ethernet */
  20,  /* header_instance_ipv4 */
  32,  /* header_instance_standard_metadata */
  -1   /* HEADER_INSTANCE_COUNT */
};

static const int header_instance_is_metadata[HEADER_INSTANCE_COUNT + 1] = {
  0,   /* header_instance_NONE */
  0,   /* header_instance_ethernet */
  0,   /* header_instance_ipv4 */
  1,   /* header_instance_standard_metadata */
  0    /* HEADER_INSTANCE_COUNT */
};

/******************************************************************************/

static const int field_instance_bit_width[FIELD_INSTANCE_COUNT + 1] = {
  -1,  /* field_instance_NONE */
  48,  /* field_instance_ethernet_dstAddr */
  48,  /* field_instance_ethernet_srcAddr */
  16,  /* field_instance_ethernet_etherType */
  4,   /* field_instance_ipv4_version */
  4,   /* field_instance_ipv4_ihl */
  8,   /* field_instance_ipv4_diffserv */
  16,  /* field_instance_ipv4_totalLen */
  16,  /* field_instance_ipv4_identification */
  3,   /* field_instance_ipv4_flags */
  13,  /* field_instance_ipv4_fragOffset */
  8,   /* field_instance_ipv4_ttl */
  8,   /* field_instance_ipv4_protocol */
  16,  /* field_instance_ipv4_hdrChecksum */
  32,  /* field_instance_ipv4_srcAddr */
  32,  /* field_instance_ipv4_dstAddr */
  9,   /* field_instance_standard_metadata_ingress_port */
  32,  /* field_instance_standard_metadata_packet_length */
  9,   /* field_instance_standard_metadata_egress_spec */
  9,   /* field_instance_standard_metadata_egress_port */
  32,  /* field_instance_standard_metadata_egress_instance */
  32,  /* field_instance_standard_metadata_instance_type */
  32,  /* field_instance_standard_metadata_clone_spec */
  5,   /* field_instance_standard_metadata__padding */
  -1   /* FIELD_INSTANCE_COUNT */
};

static const int field_instance_bit_offset[FIELD_INSTANCE_COUNT + 1] = {
  -1,  /* field_instance_NONE */
  0,   /* field_instance_ethernet_dstAddr */
  0,   /* field_instance_ethernet_srcAddr */
  0,   /* field_instance_ethernet_etherType */
  0,   /* field_instance_ipv4_version */
  4,   /* field_instance_ipv4_ihl */
  0,   /* field_instance_ipv4_diffserv */
  0,   /* field_instance_ipv4_totalLen */
  0,   /* field_instance_ipv4_identification */
  0,   /* field_instance_ipv4_flags */
  3,   /* field_instance_ipv4_fragOffset */
  0,   /* field_instance_ipv4_ttl */
  0,   /* field_instance_ipv4_protocol */
  0,   /* field_instance_ipv4_hdrChecksum */
  0,   /* field_instance_ipv4_srcAddr */
  0,   /* field_instance_ipv4_dstAddr */
  0,   /* field_instance_standard_metadata_ingress_port */
  1,   /* field_instance_standard_metadata_packet_length */
  1,   /* field_instance_standard_metadata_egress_spec */
  2,   /* field_instance_standard_metadata_egress_port */
  3,   /* field_instance_standard_metadata_egress_instance */
  3,   /* field_instance_standard_metadata_instance_type */
  3,   /* field_instance_standard_metadata_clone_spec */
  3,   /* field_instance_standard_metadata__padding */
  -1   /* FIELD_INSTANCE_COUNT */
};

static const int field_instance_byte_offset_hdr[FIELD_INSTANCE_COUNT + 1] = {
  -1,  /* field_instance_NONE */
  0,   /* field_instance_ethernet_dstAddr */
  6,   /* field_instance_ethernet_srcAddr */
  12,  /* field_instance_ethernet_etherType */
  0,   /* field_instance_ipv4_version */
  0,   /* field_instance_ipv4_ihl */
  1,   /* field_instance_ipv4_diffserv */
  2,   /* field_instance_ipv4_totalLen */
  4,   /* field_instance_ipv4_identification */
  6,   /* field_instance_ipv4_flags */
  6,   /* field_instance_ipv4_fragOffset */
  8,   /* field_instance_ipv4_ttl */
  9,   /* field_instance_ipv4_protocol */
  10,  /* field_instance_ipv4_hdrChecksum */
  12,  /* field_instance_ipv4_srcAddr */
  16,  /* field_instance_ipv4_dstAddr */
  0,   /* field_instance_standard_metadata_ingress_port */
  1,   /* field_instance_standard_metadata_packet_length */
  5,   /* field_instance_standard_metadata_egress_spec */
  6,   /* field_instance_standard_metadata_egress_port */
  7,   /* field_instance_standard_metadata_egress_instance */
  11,  /* field_instance_standard_metadata_instance_type */
  15,  /* field_instance_standard_metadata_clone_spec */
  19,  /* field_instance_standard_metadata__padding */
  -1   /* FIELD_INSTANCE_COUNT */
};

static const uint32_t field_instance_mask[FIELD_INSTANCE_COUNT + 1] = {
  -1,  /* field_instance_NONE */
  0x0,         /* field_instance_ethernet_dstAddr */
  0x0,         /* field_instance_ethernet_srcAddr */
  0xffff,  /* field_instance_ethernet_etherType */
  0xf,   /* field_instance_ipv4_version */
  0xf0,   /* field_instance_ipv4_ihl */
  0xff,  /* field_instance_ipv4_diffserv */
  0xffff,  /* field_instance_ipv4_totalLen */
  0xffff,  /* field_instance_ipv4_identification */
  0x7,   /* field_instance_ipv4_flags */
  0xfff8,  /* field_instance_ipv4_fragOffset */
  0xff,  /* field_instance_ipv4_ttl */
  0xff,  /* field_instance_ipv4_protocol */
  0xffff,  /* field_instance_ipv4_hdrChecksum */
  0xffffffff,  /* field_instance_ipv4_srcAddr */
  0xffffffff,  /* field_instance_ipv4_dstAddr */
  0x1ff,   /* field_instance_standard_metadata_ingress_port */
  0xffffff,   /* field_instance_standard_metadata_packet_length */
  0x3fe,   /* field_instance_standard_metadata_egress_spec */
  0x7fc,   /* field_instance_standard_metadata_egress_port */
  0xffffff,   /* field_instance_standard_metadata_egress_instance */
  0xffffff,  /* field_instance_standard_metadata_instance_type */
  0xffffff,  /* field_instance_standard_metadata_clone_spec */
  0xf8,  /* field_instance_standard_metadata__padding */
  -1   /* FIELD_INSTANCE_COUNT */
};

static const header_instance_t field_instance_header[FIELD_INSTANCE_COUNT + 1] = {
  header_instance_NONE,                /* field_instance_NONE */
  header_instance_ethernet,            /* field_instance_ethernet_dstAddr */
  header_instance_ethernet,            /* field_instance_ethernet_srcAddr */
  header_instance_ethernet,            /* field_instance_ethernet_etherType */
  header_instance_ipv4,                /* field_instance_ipv4_version */
  header_instance_ipv4,                /* field_instance_ipv4_ihl */
  header_instance_ipv4,                /* field_instance_ipv4_diffserv */
  header_instance_ipv4,                /* field_instance_ipv4_totalLen */
  header_instance_ipv4,                /* field_instance_ipv4_identification */
  header_instance_ipv4,                /* field_instance_ipv4_flags */
  header_instance_ipv4,                /* field_instance_ipv4_fragOffset */
  header_instance_ipv4,                /* field_instance_ipv4_ttl */
  header_instance_ipv4,                /* field_instance_ipv4_protocol */
  header_instance_ipv4,                /* field_instance_ipv4_hdrChecksum */
  header_instance_ipv4,                /* field_instance_ipv4_srcAddr */
  header_instance_ipv4,                /* field_instance_ipv4_dstAddr */
  header_instance_standard_metadata,   /* field_instance_standard_metadata_ingress_port */
  header_instance_standard_metadata,   /* field_instance_standard_metadata_packet_length */
  header_instance_standard_metadata,   /* field_instance_standard_metadata_egress_spec */
  header_instance_standard_metadata,   /* field_instance_standard_metadata_egress_port */
  header_instance_standard_metadata,   /* field_instance_standard_metadata_egress_instance */
  header_instance_standard_metadata,   /* field_instance_standard_metadata_instance_type */
  header_instance_standard_metadata,   /* field_instance_standard_metadata_clone_spec */
  header_instance_standard_metadata,   /* field_instance_standard_metadata__padding */
  header_instance_NONE                 /* FIELD_INSTANCE_COUNT */
};

#endif // __HEADER_INFO_H__
