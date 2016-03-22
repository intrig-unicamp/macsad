#include "dpdk_lib.h"

static void
parse_header(uint8_t* buffer, packet_descriptor_t* packet_desc, header_instance_t h)
{
    packet_desc->headers[h] =
      (header_descriptor_t) {
        .type = h,
        .pointer = buffer,
        .length = header_instance_byte_width[h]
      };
}

void
parse_packet(packet_descriptor_t* packet_desc)
{
    uint8_t* buffer = packet_desc->pointer;
    parse_header(buffer, packet_desc, header_instance_ethernet);
    buffer += header_instance_byte_width[header_instance_ethernet];
    parse_header(buffer, packet_desc, header_instance_ipv4);
}

