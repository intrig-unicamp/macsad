#ifndef __HEADER_INFO_H__
#define __HEADER_INFO_H__

#define HEADER_INSTANCE_COUNT 2+2
#define FIELD_INSTANCE_COUNT 4+9

enum header_instance_e {
    header_instance_NONE,
    /* virtual must go first */
    header_instance_whatever,
    header_instance_whatever2,
    header_instance_standard_metadata,
    header_instance_COUNT
};

enum field_instance_e {
    field_instance_NONE,
    /* virtual must go first */
    field_instance_whatever_field1,
    field_instance_whatever_field2,
    field_instance_whatever2_field3,
    field_instance_whatever2_field4,

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
  4,   /* header_instance_whatever */
  4,   /* header_instance_whatever2 */
  32,  /* header_instance_standard_metadata */
  -1   /* HEADER_INSTANCE_COUNT */
};

static const int header_instance_is_metadata[HEADER_INSTANCE_COUNT + 1] = {
  0,   /* header_instance_NONE */
  1,   /* header_instance_whatever */
  1,   /* header_instance_whatever2 */
  1,   /* header_instance_standard_metadata */
  0    /* HEADER_INSTANCE_COUNT */
};

/******************************************************************************/

static const int field_instance_bit_width[FIELD_INSTANCE_COUNT + 1] = {
  -1,  /* field_instance_NONE */
  16,  /* field_instance_whatever_field1 */
  16,  /* field_instance_whatever_field2 */
  11,  /* field_instance_whatever2_field3 */
  21,  /* field_instance_whatever2_field4 */
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
  0,   /* field_instance_whatever_field1 */
  0,   /* field_instance_whatever_field2 */
  0,   /* field_instance_whatever2_field3 */
  3,   /* field_instance_whatever2_field4 */
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
  0,   /* field_instance_whatever_field1 */
  2,   /* field_instance_whatever_field2 */
  0,   /* field_instance_whatever2_field3 */
  2,   /* field_instance_whatever2_field4 */
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
  0x0000FFFF,   /* field_instance_whatever_field1 */
  0x0000FFFF,   /* field_instance_whatever_field2 */
  0x000007FF,   /* field_instance_whatever2_field3 */
  0x00FFFFF8,   /* field_instance_whatever2_field4 */
  0x000001FF,   /* field_instance_standard_metadata_ingress_port */
  0,   /* field_instance_standard_metadata_packet_length */
  0,   /* field_instance_standard_metadata_egress_spec */
  0,   /* field_instance_standard_metadata_egress_port */
  0,   /* field_instance_standard_metadata_egress_instance */
  0,  /* field_instance_standard_metadata_instance_type */
  0,  /* field_instance_standard_metadata_clone_spec */
  0,  /* field_instance_standard_metadata__padding */
  -1   /* FIELD_INSTANCE_COUNT */
};

static const header_instance_t field_instance_header[FIELD_INSTANCE_COUNT + 1] = {
  header_instance_NONE,                /* field_instance_NONE */
  header_instance_whatever,            /* field_instance_whatever_field1 */
  header_instance_whatever,            /* field_instance_whatever_field2 */
  header_instance_whatever2,           /* field_instance_whatever2_field3 */
  header_instance_whatever2,           /* field_instance_whatever2_field4 */
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
